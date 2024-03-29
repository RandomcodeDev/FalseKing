/// Default file extension for VPK files
pub const VPK_EXTENSION: &str = "vpk";

/// Based on my C++ implementation: https://github.com/RandomcodeDev/FalseKing/blob/e6f62531b80fbb3a47bf83aca238c16892aff9ed/core/vpk.cpp
/// Overall, functional without significant issues. Could be improved, but likely doesn't need to be yet.
pub mod vpk2 {
    use crate::fs::{DirEntry, FileKind, FileSystem, FileType, Metadata, Permissions};
    use binary_serde::{BinarySerde, Endianness};
    use log::{debug, error, info, warn};
    use std::{
        collections::HashMap,
        fs::{self, OpenOptions},
        io::{self, Write},
        mem,
        path::{Path, PathBuf},
        sync::{Arc, Mutex},
        time::SystemTime,
    };

    /// Signature of VPK version 2 header
    const VPK2_SIGNATURE: u32 = 0x55AA1234;

    /// Version number of VPK 2
    const VPK2_VERSION: u32 = 2;

    /// Special archive index for entries stored in the directory file
    const _VPK2_SPECIAL_INDEX: u16 = 0x7FFF;

    /// Maximum size of an archive (200M)
    const VPK2_CHUNK_MAX_SIZE: usize = 209715200;

    /// Header of a VPK 2 directory file
    #[repr(C)]
    #[derive(Clone, Debug, BinarySerde)]
    struct Vpk2Header {
        /// Signature, must equal `VPK2_SIGNATURE`
        signature: u32,
        /// Version number, must equal `VPK2_VERSION`
        version: u32,
        /// Size of the directory tree
        tree_size: u32,
        /// Size of data stored in the archive
        file_data_size: u32,
        /// Size of the external MD5 section
        external_md5_size: u32,
        /// Size of the MD5 data
        md5_size: u32,
        /// Size of the signature section
        signature_size: u32,
    }

    impl Default for Vpk2Header {
        fn default() -> Self {
            Self {
                signature: VPK2_SIGNATURE,
                version: VPK2_VERSION,
                tree_size: 0,
                file_data_size: 0,
                external_md5_size: 0,
                md5_size: mem::size_of::<Vpk2Md5>() as u32,
                signature_size: 0,
            }
        }
    }

    impl Vpk2Header {
        pub fn is_valid(&self) -> bool {
            self.signature == VPK2_SIGNATURE
        }

        pub fn is_correct_version(&self) -> bool {
            self.version == VPK2_VERSION
        }
    }

    const VPK2_ENTRY_TERMINATOR: u16 = 0xFFFF;

    /// Directory entry of a VPK 2 file
    #[repr(C)]
    #[derive(Clone, Debug, BinarySerde)]
    struct Vpk2DirectoryEntry {
        /// Special Valve CRC32
        crc: u32,
        /// Amount of data stored in the directory file
        preload_size: u16,
        /// Index of the archive file where the file is stored, or
        /// `VPK2_SPECIAL_INDEX` if it's in the directory file
        archive_index: u16,
        /// Offset of the file in the archive
        offset: u32,
        /// Length of the file
        length: u32,
        /// Must equal `VPK2_ENTRY_TERMINATOR`
        terminator: u16,
    }

    impl Default for Vpk2DirectoryEntry {
        fn default() -> Self {
            Self {
                crc: 0,
                preload_size: 0,
                archive_index: 0,
                offset: 0,
                length: 0,
                terminator: VPK2_ENTRY_TERMINATOR,
            }
        }
    }

    impl From<Vpk2DirectoryEntry> for Metadata {
        fn from(value: Vpk2DirectoryEntry) -> Self {
            Self::new(
                FileType(FileKind::Regular),
                false,
                true,
                false,
                value.length as u64,
                Permissions(false),
                SystemTime::now(),
                SystemTime::now(),
                SystemTime::now(),
            )
        }
    }

    /// MD5 hashe of a file in an archive
    #[repr(C)]
    #[derive(Clone, Debug, BinarySerde)]
    struct Vpk2ExternalMd5Entry {
        /// The index where the data is
        archive_index: u32,
        /// The offset of the data
        starting_offset: u32,
        /// The length of the data
        count: u32,
        /// MD5 hash
        md5: u128,
    }

    /// MD5 hashes of the directory file
    #[repr(C)]
    #[derive(Clone, Debug, Default, BinarySerde)]
    struct Vpk2Md5 {
        /// Hash of the directory tree
        tree_md5: u128,
        /// Hash of the external MD5 section
        external_md5_md5: u128,
        /// Unknown
        unknown: u128,
    }

    /// Signature section (TODO: make serializable, even though it's not understood well enough to use)
    #[repr(C)]
    #[derive(Clone, Debug, Default)]
    struct Vpk2Signature {
        /// Public key
        _public_key: Vec<u8>,
        /// Signature
        _signature: Vec<u8>,
    }

    /// VPK version 2
    #[derive(Clone)]
    pub struct Vpk2 {
        real_path: PathBuf,

        header: Vpk2Header,
        external_md5_section: Vec<Vpk2ExternalMd5Entry>,
        md5: Vpk2Md5,
        //signature: Vpk2Signature,
        files: Arc<Mutex<HashMap<String, Vpk2DirectoryEntry>>>,

        current_archive: u16,
        current_offset: u32,
    }

    impl Vpk2 {
        pub fn new(path: &Path, create: bool) -> Option<Self> {
            let mut self_ = Self::default();

            self_.real_path = PathBuf::from(path);

            if create {
                info!("Creating VPK file {}", self_.real_path.display());
                return Some(self_);
            } else {
                info!("Loading VPK file {}", self_.real_path.display());
            }

            let dir_path = self_.get_directory_path();

            let directory = match fs::read(&dir_path) {
                Ok(directory) => directory,
                Err(err) => {
                    error!(
                        "Failed to read VPK directory {}: {err} ({err:?})",
                        dir_path.display()
                    );
                    return None;
                }
            };
            if directory.len() < mem::size_of::<Vpk2Header>() {
                error!(
                    "VPK directory is {} byte(s), should be at least {} bytes",
                    directory.len(),
                    mem::size_of::<Vpk2Header>()
                );
                return None;
            }

            self_.header =
                match Vpk2Header::binary_deserialize(directory.as_ref(), Endianness::Little) {
                    Ok(header) => header,
                    Err(err) => {
                        error!(
                            "Failed to parse VPK header in {}: {err} ({err:?})",
                            dir_path.display()
                        );
                        return None;
                    }
                };

            if !self_.header.is_valid() {
                let signature = self_.header.signature;
                error!(
                    "VPK signature of {} is 0x{:08X}, expected 0x{VPK2_SIGNATURE:08X}",
                    dir_path.display(),
                    signature
                );
                return None;
            }

            if !self_.header.is_correct_version() {
                let version = self_.header.version;
                error!(
                    "VPK {} is version {}, should be {VPK2_VERSION}",
                    dir_path.display(),
                    version
                );
                return None;
            }

            let mut current_offset = mem::size_of::<Vpk2Header>();

            // The directory tree is just strings, this is useful
            let read_string = |current_offset: &mut usize, directory: &Vec<u8>| -> String {
                let mut string = String::new();

                while directory[*current_offset] != 0 {
                    string.push(directory[*current_offset] as char);
                    *current_offset += 1;
                }

                // NUL terminator
                *current_offset += 1;

                string
            };

            let mut files = self_.files.lock().unwrap();

            loop {
                let extension = read_string(&mut current_offset, &directory);
                if extension.len() < 1 {
                    break;
                }

                loop {
                    let path = read_string(&mut current_offset, &directory);
                    if path.len() < 1 {
                        break;
                    }

                    loop {
                        let name = read_string(&mut current_offset, &directory);
                        if name.len() < 1 {
                            break;
                        }

                        let mut full_path = String::new();
                        if path != " " {
                            full_path += &path;
                            full_path += "/";
                        }
                        if name != " " {
                            full_path += &name;
                        }
                        if extension != " " {
                            full_path += ".";
                            full_path += &extension;
                        }

                        files.insert(
                            full_path.clone(),
                            match Vpk2DirectoryEntry::binary_deserialize(
                                &directory[current_offset
                                    ..current_offset + mem::size_of::<Vpk2DirectoryEntry>()],
                                Endianness::Little
                            ) {
                                Ok(entry) => entry,
                                Err(err) => {
                                    error!(
                                    "Failed to get directory entry for {full_path} in {}: {err} ({err:?})",
                                    dir_path.display()
                                );
                                    return None;
                                }
                            },
                        );

                        if files[&full_path].offset > self_.current_offset {
                            self_.current_offset = files[&full_path].offset;
                        }
                        if files[&full_path].archive_index > self_.current_archive {
                            self_.current_archive = files[&full_path].archive_index;
                            self_.current_offset = files[&full_path].offset;
                        }

                        self_.current_offset = (mem::size_of::<Vpk2DirectoryEntry>()
                            + files[&full_path].preload_size as usize)
                            as u32;
                        debug!("Got entry {full_path} in {}", dir_path.display());
                    }
                }
            }

            current_offset += self_.header.file_data_size as usize;

            let md5_entry_count =
                self_.header.external_md5_size as usize / mem::size_of::<Vpk2ExternalMd5Entry>();
            self_.external_md5_section.reserve(md5_entry_count);
            for i in 0..md5_entry_count {
                let start = current_offset + i * mem::size_of::<Vpk2ExternalMd5Entry>();
                let end = current_offset + (i + 1) * mem::size_of::<Vpk2ExternalMd5Entry>();
                self_.external_md5_section.push(
                    match Vpk2ExternalMd5Entry::binary_deserialize(
                        &directory[start..end],
                        Endianness::Little,
                    ) {
                        Ok(entry) => entry,
                        Err(err) => {
                            error!(
                                "Failed to parse MD5 entry in {}: {err} ({err:?})",
                                dir_path.display()
                            );
                            return None;
                        }
                    },
                );
            }
            current_offset += self_.header.external_md5_size as usize;

            if self_.header.md5_size as usize >= mem::size_of::<Vpk2Md5>() {
                self_.md5 = match Vpk2Md5::binary_deserialize(
                    &directory[current_offset..current_offset + mem::size_of::<Vpk2Md5>()],
                    Endianness::Little,
                ) {
                    Ok(md5) => md5,
                    Err(err) => {
                        error!(
                            "Failed to parse directory MD5 in {}: {err} ({err:?})",
                            dir_path.display()
                        );
                        return None;
                    }
                };
                //current_offset += self_.header.md5_size as usize;
            }

            /*if self_.header.signature_size as usize >= mem::size_of::<Vpk2Signature>() {
                self_.signature = match serde_binary::from_slice(&directory
                    [current_offset..current_offset + mem::size_of::<Vpk2Signature>()], Endian::Little)
                {
                    Ok(md5) => md5,
                    Err(err) => {
                        error!(
                            "Failed to parse directory MD5 in {}: {err} ({err:?})",
                            dir_path.display()
                        );
                        return None;
                    }
                };
                //current_offset += self_.header.signature_size as usize;
            }*/

            Some(self_.clone())
        }

        pub fn save(&mut self, path: &Path) -> io::Result<()> {
            let have_path = self.real_path.as_os_str().len() > 0;
            let have_new_path = path.as_os_str().len() > 0;
            if !have_path && !have_new_path {
                warn!("Not writing VPK file without a name");
            }

            if !have_path || have_new_path {
                self.real_path = PathBuf::from(path);
                self.real_path.set_extension(super::VPK_EXTENSION);
            }

            info!("Saving VPK file to {}", self.real_path.display());

            let mut name_tree: HashMap<
                String,
                HashMap<String, HashMap<String, Vpk2DirectoryEntry>>,
            > = HashMap::new();

            let files = self.files.lock().unwrap();
            files.iter().for_each(|(full_path, entry)| {
                let mut name = String::new();
                let extension = String::from(if let Some(last_dot) = full_path.find('.') {
                    let split = full_path.split_at(last_dot);
                    name = String::from(split.0);
                    split.1.strip_prefix('.').unwrap() // remove first .
                } else {
                    " "
                });
                let path = String::from(if let Some(last_slash) = full_path.rfind('/') {
                    name = String::from(name.split_at(last_slash).1);
                    name.remove(0); // remove first /
                    full_path.split_at(last_slash).0
                } else {
                    " "
                });

                if name.len() < 1 {
                    name.push(' ');
                }

                if let Some(paths) = name_tree.get_mut(&extension) {
                    if let Some(names) = paths.get_mut(&path) {
                        names.insert(name, entry.clone());
                    }
                } else {
                    // unwrap is fine because everything has just been inserted
                    name_tree.insert(extension.clone(), HashMap::new());
                    let paths = name_tree.get_mut(&extension).unwrap();
                    paths.insert(path.clone(), HashMap::new());
                    let names = paths.get_mut(&path).unwrap();
                    names.insert(name.clone(), entry.clone());
                }
            });

            let mut directory: Vec<u8> = Vec::new();
            directory.resize(mem::size_of::<Vpk2Header>(), 0);

            name_tree.iter().for_each(|(extension, paths)| {
                let mut extension_raw = Vec::from(extension.as_str().as_bytes());
                extension_raw.push(0);
                directory.append(&mut extension_raw);

                paths.iter().for_each(|(path, names)| {
                    let mut path_raw = Vec::from(path.as_str().as_bytes());
                    path_raw.push(0);
                    directory.append(&mut path_raw);

                    names.iter().for_each(|(name, entry)| {
                        let mut name_raw = Vec::from(name.as_str().as_bytes());
                        name_raw.push(0);
                        directory.append(&mut name_raw);
                        let len = directory.len();
                        directory.resize(directory.len() + mem::size_of::<Vpk2DirectoryEntry>(), 0);
                        entry.binary_serialize(&mut directory[len..], Endianness::Little);

                        // This is _probably_ because the struct has 3 u16's, which makes something
                        // somewhere align it, causing two zeroes that shouldn't be there to prematurely
                        // end this level of the tree
                        directory.resize(directory.len() - 2, 0);
                    });

                    directory.push(0);
                });

                directory.push(0);
            });

            directory.push(0);

            self.header.tree_size = (directory.len() - mem::size_of::<Vpk2Header>()) as u32;
            self.header.binary_serialize(
                &mut directory[..mem::size_of::<Vpk2Header>()],
                Endianness::Little,
            );
            directory.resize(directory.len() + mem::size_of::<Vpk2Md5>(), 0);
            self.md5.binary_serialize(
                &mut directory[mem::size_of::<Vpk2Header>() + self.header.tree_size as usize..],
                Endianness::Little,
            );

            let dir_path = self.get_directory_path();
            let tree_size = self.header.tree_size;
            info!(
                "Writing VPK directory to {}. Directory tree is {} byte(s), directory is {} byte(s).",
                dir_path.display(),
                tree_size,
                directory.len()
            );

            fs::write(dir_path, directory)?;

            Ok(())
        }

        fn get_directory_path(&self) -> PathBuf {
            format!(
                "{}_dir.{}",
                self.real_path.with_extension("").display(),
                super::VPK_EXTENSION
            )
            .into()
        }

        fn get_archive_path(&self, index: Option<u32>) -> PathBuf {
            format!(
                "{}_{:03}.{}",
                self.real_path.with_extension("").display(),
                if index.is_some() {
                    index.unwrap()
                } else {
                    self.current_archive as u32
                },
                super::VPK_EXTENSION
            )
            .into()
        }

        pub fn get_base_path(path: &Path) -> io::Result<PathBuf> {
            let path = String::from(
                path.as_os_str()
                    .to_str()
                    .ok_or(io::Error::from(io::ErrorKind::Other))?,
            );
            let mut split = path.rsplit('_');
            split.next();
            let mut name = PathBuf::from(split.next().unwrap_or_default());
            name.set_extension(super::VPK_EXTENSION);
            Ok(name)
        }
    }

    pub struct ReadDir {
        files: Arc<Mutex<HashMap<String, Vpk2DirectoryEntry>>>,
        index: usize,
    }

    impl Iterator for ReadDir {
        type Item = io::Result<DirEntry>;

        fn next(&mut self) -> Option<Self::Item> {
            let files = self.files.lock().unwrap();
            let entry = files.iter().nth(self.index)?;
            self.index += 1;
            let metadata: Metadata = entry.1.clone().into();
            info!("{:?}", entry.1);
            Some(Ok(DirEntry::new(
                PathBuf::from(entry.0),
                metadata.clone(),
                metadata.file_type(),
            )))
        }
    }

    impl FileSystem for Vpk2 {
        fn try_exists(&self, path: &Path) -> io::Result<bool> {
            let path = String::from(
                path.as_os_str()
                    .to_str()
                    .ok_or(io::Error::from(io::ErrorKind::Other))?,
            );
            let files = self.files.lock().unwrap();
            Ok(files.get(&path).is_some())
        }

        fn canonicalize(&self, path: &Path) -> io::Result<PathBuf> {
            Ok(PathBuf::from(format!(
                "{}.{}/{}",
                self.real_path.display(),
                super::VPK_EXTENSION,
                path.display()
            )))
        }

        fn copy(&mut self, from: &Path, to: &Path) -> io::Result<u64> {
            let from = String::from(
                from.as_os_str()
                    .to_str()
                    .ok_or(io::Error::from(io::ErrorKind::Other))?,
            );
            let to = String::from(
                to.as_os_str()
                    .to_str()
                    .ok_or(io::Error::from(io::ErrorKind::Other))?,
            );

            let mut files_mut = self.files.lock().unwrap();
            let files = self.files.lock().unwrap();
            files_mut.insert(to, files[&from].clone());

            Ok(0)
        }

        fn create_dir(&mut self, _path: &Path) -> io::Result<()> {
            // Creating directories doesn't matter in VPKs
            Err(io::Error::from(io::ErrorKind::Unsupported))
        }

        fn create_dir_all(&mut self, _path: &Path) -> io::Result<()> {
            // Creating directories doesn't matter in VPKs
            Err(io::Error::from(io::ErrorKind::Unsupported))
        }

        fn hard_link(&mut self, original: &Path, link: &Path) -> io::Result<()> {
            self.copy(original, link)?;
            Ok(())
        }

        fn metadata(&self, path: &Path) -> io::Result<crate::fs::Metadata> {
            let path = String::from(
                path.as_os_str()
                    .to_str()
                    .ok_or(io::Error::from(io::ErrorKind::Other))?,
            );

            let files = self.files.lock().unwrap();
            if let Some(entry) = files.get(&path) {
                Ok(entry.clone().into())
            } else {
                Err(io::Error::from(io::ErrorKind::NotFound))
            }
        }

        fn read(&self, path: &Path) -> io::Result<Vec<u8>> {
            let path = String::from(
                path.as_os_str()
                    .to_str()
                    .ok_or(io::Error::from(io::ErrorKind::Other))?,
            );

            let files = self.files.lock().unwrap();
            // TODO: Same issue as C++, can a file be in more than one chunk?
            if let Some(entry) = files.get(&path) {
                let archive_path = self.get_archive_path(Some(entry.archive_index as u32));
                let archive = fs::read(&archive_path)?;
                if entry.offset as usize + entry.length as usize <= archive.len() {
                    return Ok(Vec::from(
                        archive
                            .split_at(entry.offset as usize)
                            .1
                            .split_at(entry.length as usize)
                            .0,
                    ));
                }
            }

            Err(io::Error::new(io::ErrorKind::NotFound, path))
        }

        type ReadDir = ReadDir;

        fn read_dir(&self, _path: &Path) -> io::Result<Self::ReadDir>
        where
            Self::ReadDir: Iterator,
        {
            Ok(Self::ReadDir {
                files: self.files.clone(),
                index: 0,
            })
        }

        fn read_link(&self, _path: &Path) -> io::Result<PathBuf> {
            // VPKs don't have links, but my lazy-ass implementation makes hard links
            // and symlinks and copying all shallow copies anyway
            Err(io::Error::from(io::ErrorKind::Unsupported))
        }

        fn read_to_string(&self, path: &Path) -> io::Result<String> {
            match String::from_utf8(self.read(path)?) {
                Ok(content) => Ok(content),
                Err(_) => Err(io::Error::from(io::ErrorKind::InvalidData)),
            }
        }

        fn remove_dir(&mut self, _path: &Path) -> io::Result<()> {
            // Removing directories doesn't matter in VPKs
            Err(io::Error::from(io::ErrorKind::Unsupported))
        }

        fn remove_dir_all(&mut self, _path: &Path) -> io::Result<()> {
            // Removing directories doesn't matter in VPKs
            Err(io::Error::from(io::ErrorKind::Unsupported))
        }

        fn remove_file(&mut self, path: &Path) -> io::Result<()> {
            let path = String::from(
                path.as_os_str()
                    .to_str()
                    .ok_or(io::Error::from(io::ErrorKind::Other))?,
            );

            let mut files = self.files.lock().unwrap();
            // TODO: should this delete the data or just the entry? VPKs should
            // just get recreated probably.
            match files.remove_entry(&path) {
                Some(_) => Ok(()),
                None => Err(io::Error::from(io::ErrorKind::NotFound)),
            }
        }

        fn rename(&mut self, to: &Path, from: &Path) -> io::Result<()> {
            let from = String::from(
                from.as_os_str()
                    .to_str()
                    .ok_or(io::Error::from(io::ErrorKind::Other))?,
            );
            let to = String::from(
                to.as_os_str()
                    .to_str()
                    .ok_or(io::Error::from(io::ErrorKind::Other))?,
            );

            let mut files = self.files.lock().unwrap();
            let entry = match files.remove_entry(&from) {
                Some(entry) => Ok(entry),
                None => Err(io::Error::from(io::ErrorKind::NotFound)),
            }?;

            files.insert(to, entry.1);

            Ok(())
        }

        fn set_permissions(&mut self, _path: &Path, _permissions: Permissions) -> io::Result<()> {
            // Permissions don't exist in VPKs
            Err(io::Error::from(io::ErrorKind::Unsupported))
        }

        fn soft_link(&mut self, from: &Path, to: &Path) -> io::Result<()> {
            self.hard_link(from, to)
        }

        fn symlink_metadata(&self, path: &Path) -> io::Result<crate::fs::Metadata> {
            self.metadata(path)
        }

        fn write(&mut self, path: &Path, contents: &[u8]) -> io::Result<()> {
            let mut files = self.files.lock().unwrap();
            let path = String::from(
                path.as_os_str()
                    .to_str()
                    .ok_or(io::Error::from(io::ErrorKind::Other))?,
            );

            let mut path = crate::util::clean_path(&path);
            if path.chars().nth(0).is_some_and(|c| c == '/') {
                path.remove(0);
            }

            let contents = contents.as_ref();

            debug!(
                "Writing {}-byte file {path} to {}",
                contents.len(),
                self.real_path.display()
            );

            if self.real_path.as_os_str().len() < 1 {
                warn!("This VPK file has no path associated with it, and no files can be added yet. Call save before writing to fix this.");
                return Err(io::Error::from(io::ErrorKind::PermissionDenied));
            }

            // go to the next archive if the file is too big
            if self.current_offset as usize + contents.len() > VPK2_CHUNK_MAX_SIZE {
                self.current_archive += 1;
                self.current_offset = 0;
            }

            let archive_path = self.get_archive_path(None);
            let mut archive_file = OpenOptions::new()
                .write(true)
                .append(true)
                .create(true)
                .open(archive_path)?;

            let entry = Vpk2DirectoryEntry {
                crc: super::valve_crc32(contents),
                preload_size: 0,
                archive_index: self.current_archive,
                offset: self.current_offset,
                length: contents.len() as u32,
                ..Default::default()
            };

            archive_file.write_all(contents)?;
            archive_file.flush()?;

            self.current_offset += entry.length;
            files.insert(path, entry);

            Ok(())
        }
    }

    impl Default for Vpk2 {
        fn default() -> Self {
            Self {
                real_path: PathBuf::new(),
                header: Vpk2Header::default(),
                external_md5_section: Vec::new(),
                md5: Vpk2Md5::default(),
                //signature: Vpk2Signature::default(),
                files: Arc::new(Mutex::new(HashMap::new())),
                current_archive: 0,
                current_offset: 0,
            }
        }
    }
}

/// CRC32 algorithm used by Valve for VPK files
pub fn valve_crc32(data: &[u8]) -> u32 {
    let mut crc: u32 = 0xFFFFFFFF;

    data.iter().for_each(|c| {
        crc = (crc >> 8) ^ VALVE_CRC32_TABLE[(*c ^ (crc & 0xFF) as u8) as usize];
    });

    !crc
}

/// Valve CRC32 table
const VALVE_CRC32_TABLE: [u32; 256] = [
    0x00000000, 0x77073096, 0xEE0E612C, 0x990951BA, 0x076DC419, 0x706AF48F, 0xE963A535, 0x9E6495A3,
    0x0EDB8832, 0x79DCB8A4, 0xE0D5E91E, 0x97D2D988, 0x09B64C2B, 0x7EB17CBD, 0xE7B82D07, 0x90BF1D91,
    0x1DB71064, 0x6AB020F2, 0xF3B97148, 0x84BE41DE, 0x1ADAD47D, 0x6DDDE4EB, 0xF4D4B551, 0x83D385C7,
    0x136C9856, 0x646BA8C0, 0xFD62F97A, 0x8A65C9EC, 0x14015C4F, 0x63066CD9, 0xFA0F3D63, 0x8D080DF5,
    0x3B6E20C8, 0x4C69105E, 0xD56041E4, 0xA2677172, 0x3C03E4D1, 0x4B04D447, 0xD20D85FD, 0xA50AB56B,
    0x35B5A8FA, 0x42B2986C, 0xDBBBC9D6, 0xACBCF940, 0x32D86CE3, 0x45DF5C75, 0xDCD60DCF, 0xABD13D59,
    0x26D930AC, 0x51DE003A, 0xC8D75180, 0xBFD06116, 0x21B4F4B5, 0x56B3C423, 0xCFBA9599, 0xB8BDA50F,
    0x2802B89E, 0x5F058808, 0xC60CD9B2, 0xB10BE924, 0x2F6F7C87, 0x58684C11, 0xC1611DAB, 0xB6662D3D,
    0x76DC4190, 0x01DB7106, 0x98D220BC, 0xEFD5102A, 0x71B18589, 0x06B6B51F, 0x9FBFE4A5, 0xE8B8D433,
    0x7807C9A2, 0x0F00F934, 0x9609A88E, 0xE10E9818, 0x7F6A0DBB, 0x086D3D2D, 0x91646C97, 0xE6635C01,
    0x6B6B51F4, 0x1C6C6162, 0x856530D8, 0xF262004E, 0x6C0695ED, 0x1B01A57B, 0x8208F4C1, 0xF50FC457,
    0x65B0D9C6, 0x12B7E950, 0x8BBEB8EA, 0xFCB9887C, 0x62DD1DDF, 0x15DA2D49, 0x8CD37CF3, 0xFBD44C65,
    0x4DB26158, 0x3AB551CE, 0xA3BC0074, 0xD4BB30E2, 0x4ADFA541, 0x3DD895D7, 0xA4D1C46D, 0xD3D6F4FB,
    0x4369E96A, 0x346ED9FC, 0xAD678846, 0xDA60B8D0, 0x44042D73, 0x33031DE5, 0xAA0A4C5F, 0xDD0D7CC9,
    0x5005713C, 0x270241AA, 0xBE0B1010, 0xC90C2086, 0x5768B525, 0x206F85B3, 0xB966D409, 0xCE61E49F,
    0x5EDEF90E, 0x29D9C998, 0xB0D09822, 0xC7D7A8B4, 0x59B33D17, 0x2EB40D81, 0xB7BD5C3B, 0xC0BA6CAD,
    0xEDB88320, 0x9ABFB3B6, 0x03B6E20C, 0x74B1D29A, 0xEAD54739, 0x9DD277AF, 0x04DB2615, 0x73DC1683,
    0xE3630B12, 0x94643B84, 0x0D6D6A3E, 0x7A6A5AA8, 0xE40ECF0B, 0x9309FF9D, 0x0A00AE27, 0x7D079EB1,
    0xF00F9344, 0x8708A3D2, 0x1E01F268, 0x6906C2FE, 0xF762575D, 0x806567CB, 0x196C3671, 0x6E6B06E7,
    0xFED41B76, 0x89D32BE0, 0x10DA7A5A, 0x67DD4ACC, 0xF9B9DF6F, 0x8EBEEFF9, 0x17B7BE43, 0x60B08ED5,
    0xD6D6A3E8, 0xA1D1937E, 0x38D8C2C4, 0x4FDFF252, 0xD1BB67F1, 0xA6BC5767, 0x3FB506DD, 0x48B2364B,
    0xD80D2BDA, 0xAF0A1B4C, 0x36034AF6, 0x41047A60, 0xDF60EFC3, 0xA867DF55, 0x316E8EEF, 0x4669BE79,
    0xCB61B38C, 0xBC66831A, 0x256FD2A0, 0x5268E236, 0xCC0C7795, 0xBB0B4703, 0x220216B9, 0x5505262F,
    0xC5BA3BBE, 0xB2BD0B28, 0x2BB45A92, 0x5CB36A04, 0xC2D7FFA7, 0xB5D0CF31, 0x2CD99E8B, 0x5BDEAE1D,
    0x9B64C2B0, 0xEC63F226, 0x756AA39C, 0x026D930A, 0x9C0906A9, 0xEB0E363F, 0x72076785, 0x05005713,
    0x95BF4A82, 0xE2B87A14, 0x7BB12BAE, 0x0CB61B38, 0x92D28E9B, 0xE5D5BE0D, 0x7CDCEFB7, 0x0BDBDF21,
    0x86D3D2D4, 0xF1D4E242, 0x68DDB3F8, 0x1FDA836E, 0x81BE16CD, 0xF6B9265B, 0x6FB077E1, 0x18B74777,
    0x88085AE6, 0xFF0F6A70, 0x66063BCA, 0x11010B5C, 0x8F659EFF, 0xF862AE69, 0x616BFFD3, 0x166CCF45,
    0xA00AE278, 0xD70DD2EE, 0x4E048354, 0x3903B3C2, 0xA7672661, 0xD06016F7, 0x4969474D, 0x3E6E77DB,
    0xAED16A4A, 0xD9D65ADC, 0x40DF0B66, 0x37D83BF0, 0xA9BCAE53, 0xDEBB9EC5, 0x47B2CF7F, 0x30B5FFE9,
    0xBDBDF21C, 0xCABAC28A, 0x53B39330, 0x24B4A3A6, 0xBAD03605, 0xCDD70693, 0x54DE5729, 0x23D967BF,
    0xB3667A2E, 0xC4614AB8, 0x5D681B02, 0x2A6F2B94, 0xB40BBE37, 0xC30C8EA1, 0x5A05DF1B, 0x2D02EF8D,
];
