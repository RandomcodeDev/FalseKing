use crate::fs::FileSystem;

/// Default file extension for VPK files
const VPK_EXTENSION: &str = ".vpk";

/// Based on my C++ implementation: https://github.com/RandomcodeDev/FalseKing/blob/e6f62531b80fbb3a47bf83aca238c16892aff9ed/core/vpk.cpp
/// Overall, functional without significant issues. Could be improved, but likely doesn't need to be yet.
pub mod vpk2 {
    use std::{
        array::TryFromSliceError,
        collections::HashMap,
        fs::OpenOptions,
        io::{self, Write},
        mem,
        path::{Path, PathBuf}, os::windows::prelude::OpenOptionsExt,
    };

    use log::{debug, error, info, warn};

    use crate::fs::{FileSystem, StdFileSystem};

    /// Signature of VPK version 2 header
    const VPK2_SIGNATURE: u32 = 0x55AA1234;

    /// Version number of VPK 2
    const VPK2_VERSION: u32 = 2;

    /// Special archive index for entries stored in the directory file
    const VPK2_SPECIAL_INDEX: u16 = 0x7FFF;

    /// Maximum size of an archive
    const VPK2_CHUNK_MAX_SIZE: usize = 209715200;

    /// Header of a VPK 2 directory file
    #[repr(C)]
    #[derive(Clone, Default, Debug)]
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

    impl Vpk2Header {
        pub fn is_valid(&self) -> bool {
            self.signature == VPK2_SIGNATURE
        }

        pub fn is_correct_version(&self) -> bool {
            self.version == VPK2_VERSION
        }
    }

    impl TryFrom<&[u8]> for Vpk2Header {
        /// Convert from a u8 slice of at least `size_of::<Vpk2Header>`
        fn try_from(value: &[u8]) -> Result<Self, Self::Error> {
            Ok(Self {
                signature: u32::from_le_bytes(*crate::util::slice_to_array(&value[0..3])?),
                version: u32::from_le_bytes(*crate::util::slice_to_array(&value[4..7])?),
                tree_size: u32::from_le_bytes(*crate::util::slice_to_array(&value[8..11])?),
                file_data_size: u32::from_le_bytes(*crate::util::slice_to_array(&value[12..15])?),
                external_md5_size: u32::from_le_bytes(*crate::util::slice_to_array(
                    &value[16..19],
                )?),
                md5_size: u32::from_le_bytes(*crate::util::slice_to_array(&value[20..23])?),
                signature_size: u32::from_le_bytes(*crate::util::slice_to_array(&value[24..27])?),
            })
        }

        type Error = TryFromSliceError;
    }

    const VPK2_ENTRY_TERMINATOR: u16 = 0xFFFF;

    /// Directory entry of a VPK 2 file
    #[repr(C)]
    #[derive(Clone, Default, Debug)]
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

    impl TryFrom<&[u8]> for Vpk2DirectoryEntry {
        /// Convert from a u8 slice of at least `size_of::<Vpk2DirectoryEntry>`
        fn try_from(value: &[u8]) -> Result<Self, Self::Error> {
            Ok(Self {
                crc: u32::from_le_bytes(*crate::util::slice_to_array(&value[0..3])?),
                preload_size: u16::from_le_bytes(*crate::util::slice_to_array(&value[4..5])?),
                archive_index: u16::from_le_bytes(*crate::util::slice_to_array(&value[6..7])?),
                offset: u32::from_le_bytes(*crate::util::slice_to_array(&value[8..11])?),
                length: u32::from_le_bytes(*crate::util::slice_to_array(&value[12..15])?),
                terminator: u16::from_le_bytes(*crate::util::slice_to_array(&value[16..17])?),
            })
        }

        type Error = TryFromSliceError;
    }

    /// MD5 hashe of a file in an archive
    #[repr(C)]
    #[derive(Clone, Default, Debug)]
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

    impl TryFrom<&[u8]> for Vpk2ExternalMd5Entry {
        /// Convert from a u8 slice of at least `size_of::<Vpk2ExternalMd5Entry>`
        fn try_from(value: &[u8]) -> Result<Self, Self::Error> {
            Ok(Self {
                archive_index: u32::from_le_bytes(*crate::util::slice_to_array(&value[0..3])?),
                starting_offset: u32::from_le_bytes(*crate::util::slice_to_array(&value[4..7])?),
                count: u32::from_le_bytes(*crate::util::slice_to_array(&value[8..11])?),
                md5: u128::from_le_bytes(*crate::util::slice_to_array(&value[12..27])?),
            })
        }

        type Error = TryFromSliceError;
    }

    /// MD5 hashes of the directory file
    #[repr(C)]
    #[derive(Clone, Default, Debug)]
    struct Vpk2Md5 {
        /// Hash of the directory tree
        tree_md5: u128,
        /// Hash of the external MD5 section
        external_md5_md5: u128,
        /// Unknown
        unknown: u128,
    }

    impl TryFrom<&[u8]> for Vpk2Md5 {
        /// Convert from a u8 slice of at least `size_of::<Vpk2Md5>`
        fn try_from(value: &[u8]) -> Result<Self, Self::Error> {
            Ok(Self {
                tree_md5: u128::from_le_bytes(*crate::util::slice_to_array(&value[0..15])?),
                external_md5_md5: u128::from_le_bytes(*crate::util::slice_to_array(
                    &value[16..31],
                )?),
                unknown: u128::from_le_bytes(*crate::util::slice_to_array(&value[32..47])?),
            })
        }

        type Error = TryFromSliceError;
    }

    /// Signature section
    #[repr(C)]
    #[derive(Clone, Default, Debug)]
    struct Vpk2Signature {
        /// Public key size
        public_key_size: u32,
        /// Public key
        public_key: Vec<u8>,
        /// Signature size
        signature_size: u32,
        signature: Vec<u8>,
    }

    impl TryFrom<&[u8]> for Vpk2Signature {
        /// Convert from a u8 slice of at least `size_of::<Vpk2Signature>`
        fn try_from(value: &[u8]) -> Result<Self, Self::Error> {
            let public_key_size = u32::from_le_bytes(*crate::util::slice_to_array(&value[0..3])?);
            let signature_size = u32::from_le_bytes(*crate::util::slice_to_array(
                &value[4 + public_key_size as usize..4 + public_key_size as usize + 3],
            )?);
            Ok(Self {
                public_key_size,
                public_key: Vec::from(&value[4..public_key_size as usize]),
                signature_size,
                signature: Vec::from(
                    &value[4 + public_key_size as usize + 4..4 + public_key_size as usize + 7],
                ),
            })
        }

        type Error = TryFromSliceError;
    }

    /// VPK version 2
    pub struct Vpk2 {
        real_path: PathBuf,

        header: Vpk2Header,
        external_md5_section: Vec<Vpk2ExternalMd5Entry>,
        md5: Vpk2Md5,
        signature: Vpk2Signature,
        files: HashMap<String, Vpk2DirectoryEntry>,

        current_archive: u16,
        current_offset: u32,
    }

    impl Vpk2 {
        pub fn new<P: AsRef<Path>>(path: P, create: bool) -> Option<Self>
        where
            PathBuf: From<P>,
        {
            let mut self_ = Self::default();

            self_.real_path = PathBuf::from(path);

            if create {
                info!("Creating VPK file {}", self_.real_path.display());
                return Some(self_);
            } else {
                info!("Loading VPK file {}", self_.real_path.display());
            }

            let dir_path = self_.get_directory_path();
            let fs = StdFileSystem::new();

            let directory = match fs.read(&dir_path) {
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

            self_.header = match Vpk2Header::try_from(directory.as_slice()) {
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
                error!(
                    "VPK signature of {} is 0x{:08X}, expected 0x{VPK2_SIGNATURE:08X}",
                    dir_path.display(),
                    self_.header.signature
                );
                return None;
            }

            if !self_.header.is_correct_version() {
                error!(
                    "VPK {} is version {}, should be {VPK2_VERSION}",
                    dir_path.display(),
                    self_.header.version
                );
                return None;
            }

            let mut current_offset = mem::size_of::<Vpk2Header>();

            // The directory tree is just strings, this is useful
            let mut read_string = |current_offset: &mut usize, directory: &Vec<u8>| -> String {
                let mut string = String::new();

                while directory[*current_offset] != 0 {
                    string.push(directory[*current_offset] as char);
                    *current_offset += 1;
                }

                // NUL terminator
                *current_offset += 1;

                string
            };

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

                        self_.files.insert(
                            full_path.clone(),
                            match Vpk2DirectoryEntry::try_from(
                                &directory[current_offset
                                    ..current_offset + mem::size_of::<Vpk2DirectoryEntry>()],
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

                        if self_.files[&full_path].offset > self_.current_offset {
                            self_.current_offset = self_.files[&full_path].offset;
                        }
                        if self_.files[&full_path].archive_index > self_.current_archive {
                            self_.current_archive = self_.files[&full_path].archive_index;
                            self_.current_offset = self_.files[&full_path].offset;
                        }

                        self_.current_offset = (mem::size_of::<Vpk2DirectoryEntry>()
                            + self_.files[&full_path].preload_size as usize)
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
                self_
                    .external_md5_section
                    .push(match (&directory[start..end]).try_into() {
                        Ok(entry) => entry,
                        Err(err) => {
                            error!(
                                "Failed to parse MD5 entry in {}: {err} ({err:?})",
                                dir_path.display()
                            );
                            return None;
                        }
                    });
            }
            current_offset += self_.header.external_md5_size as usize;

            if self_.header.md5_size as usize >= mem::size_of::<Vpk2Md5>() {
                self_.md5 = match (&directory
                    [current_offset..current_offset + mem::size_of::<Vpk2Md5>()])
                    .try_into()
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
                current_offset += self_.header.md5_size as usize;
            }

            if self_.header.signature_size as usize >= mem::size_of::<Vpk2Signature>() {
                self_.signature = match (&directory
                    [current_offset..current_offset + mem::size_of::<Vpk2Signature>()])
                    .try_into()
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
                current_offset += self_.header.signature_size as usize;
            }

            Some(self_)
        }

        pub fn save<P: AsRef<Path>>(&mut self, path: P) -> io::Result<()> {
            let have_path = self.real_path.as_os_str().len() > 0;
            let have_new_path = path.as_ref().as_os_str().len() > 0;
            if !have_path && !have_new_path {
                warn!("Not writing VPK file without a name");
            }

            if !have_path || have_new_path {
                self.real_path = PathBuf::from(path.as_ref());
                self.real_path.set_extension(super::VPK_EXTENSION);
            }

            info!("Saving VPK file to {}", self.real_path.display());

            let dir_path = self.get_directory_path();
            //let 

            Ok(())
        }

        fn get_directory_path(&self) -> PathBuf {
            format!("{}_dir.{}", self.real_path.display(), super::VPK_EXTENSION).into()
        }

        fn get_archive_path(&self, index: Option<u32>) -> PathBuf {
            format!(
                "{}_{:.3}.{}",
                self.real_path.display(),
                if index.is_some() {
                    index.unwrap()
                } else {
                    self.current_archive as u32
                },
                super::VPK_EXTENSION
            )
            .into()
        }
    }

    impl FileSystem for Vpk2 {
        fn try_exists<P: AsRef<Path>>(&self, path: P) -> io::Result<bool> {
            let path = String::from(
                path.as_ref()
                    .as_os_str()
                    .to_str()
                    .ok_or(io::Error::from(io::ErrorKind::Other))?,
            );
            Ok(self.files.get(&path).is_some())
        }

        fn canonicalize<P: AsRef<Path>>(&self, path: P) -> io::Result<PathBuf> {
            Ok(PathBuf::from(format!(
                "{}.{}/{}",
                self.real_path.display(),
                super::VPK_EXTENSION,
                path.as_ref().display()
            )))
        }

        fn copy<P: AsRef<Path>, Q: AsRef<Path>>(&mut self, from: P, to: Q) -> io::Result<u64> {
            let from = String::from(
                from.as_ref()
                    .as_os_str()
                    .to_str()
                    .ok_or(io::Error::from(io::ErrorKind::Other))?,
            );

            let to = String::from(
                to.as_ref()
                    .as_os_str()
                    .to_str()
                    .ok_or(io::Error::from(io::ErrorKind::Other))?,
            );
            self.files.insert(to, self.files[&from].clone());

            Ok(0)
        }

        fn create_dir<P: AsRef<Path>>(&mut self, path: P) -> io::Result<()> {
            // Creating directories doesn't matter in VPKs
            Err(io::Error::from(io::ErrorKind::Unsupported))
        }

        fn create_dir_all<P: AsRef<Path>>(&mut self, path: P) -> io::Result<()> {
            // Creating directories doesn't matter in VPKs
            Err(io::Error::from(io::ErrorKind::Unsupported))
        }

        fn hard_link<P: AsRef<Path>, Q: AsRef<Path>>(
            &mut self,
            original: P,
            link: Q,
        ) -> io::Result<()> {
            self.copy(original, link)?;
            Ok(())
        }

        fn metadata<P: AsRef<Path>>(&self, path: P) -> io::Result<crate::fs::Metadata> {
            todo!()
        }

        fn read<P: AsRef<Path>>(&self, path: P) -> io::Result<Vec<u8>> {
            let path = String::from(
                path.as_ref()
                    .as_os_str()
                    .to_str()
                    .ok_or(io::Error::from(io::ErrorKind::Other))?,
            );

            // TODO: Same issue as C++, can a file be in more than one chunk?
            if let Some(entry) = self.files.get(&path) {
                let fs = StdFileSystem::new();
                let archive_path = self.get_archive_path(Some(entry.archive_index as u32));
                let archive = fs.read(&archive_path)?;
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

        fn read_dir<P: AsRef<Path>>(
            &self,
            path: P,
        ) -> io::Result<Box<dyn Iterator<Item = io::Result<crate::fs::DirEntry>>>> {
            todo!()
        }

        fn read_link<P: AsRef<Path>>(&self, path: P) -> io::Result<PathBuf> {
            // VPKs don't have links, but my lazy-ass implementation makes hard links
            // and symlinks and copying all shallow copies anyway
            Err(io::Error::from(io::ErrorKind::Unsupported))
        }

        fn read_to_string<P: AsRef<Path>>(&self, path: P) -> io::Result<String> {
            match String::from_utf8(self.read(path)?) {
                Ok(content) => Ok(content),
                Err(_) => Err(io::Error::from(io::ErrorKind::InvalidData)),
            }
        }

        fn remove_dir<P: AsRef<Path>>(&mut self, path: P) -> io::Result<()> {
            // Removing directories doesn't matter in VPKs
            Err(io::Error::from(io::ErrorKind::Unsupported))
        }

        fn remove_dir_all<P: AsRef<Path>>(&mut self, path: P) -> io::Result<()> {
            // Removing directories doesn't matter in VPKs
            Err(io::Error::from(io::ErrorKind::Unsupported))
        }

        fn remove_file<P: AsRef<Path>>(&mut self, path: P) -> io::Result<()> {
            let path = String::from(
                path.as_ref()
                    .as_os_str()
                    .to_str()
                    .ok_or(io::Error::from(io::ErrorKind::Other))?,
            );

            // TODO: should this delete the data or just the entry? VPKs should
            // just get recreated probably.
            match self.files.remove_entry(&path) {
                Some(_) => Ok(()),
                None => Err(io::Error::from(io::ErrorKind::NotFound)),
            }
        }

        fn rename<P: AsRef<Path>, Q: AsRef<Path>>(&mut self, to: P, from: Q) -> io::Result<()> {
            let from = String::from(
                from.as_ref()
                    .as_os_str()
                    .to_str()
                    .ok_or(io::Error::from(io::ErrorKind::Other))?,
            );
            let to = String::from(
                to.as_ref()
                    .as_os_str()
                    .to_str()
                    .ok_or(io::Error::from(io::ErrorKind::Other))?,
            );

            let entry = match self.files.remove_entry(&from) {
                Some(entry) => Ok(entry),
                None => Err(io::Error::from(io::ErrorKind::NotFound)),
            }?;

            self.files.insert(to, entry.1);

            Ok(())
        }

        fn set_permissions<P: AsRef<Path>>(
            &mut self,
            path: P,
            permissions: std::fs::Permissions,
        ) -> io::Result<()> {
            // Permissions don't exist in VPKs
            Err(io::Error::from(io::ErrorKind::Unsupported))
        }

        fn soft_link<P: AsRef<Path>, Q: AsRef<Path>>(&mut self, from: P, to: Q) -> io::Result<()> {
            self.hard_link(from, to)
        }

        fn symlink_metadata<P: AsRef<Path>>(&self, path: P) -> io::Result<crate::fs::Metadata> {
            self.metadata(path)
        }

        fn write<P: AsRef<Path>, C: AsRef<[u8]>>(
            &mut self,
            path: P,
            contents: C,
        ) -> io::Result<()> {
            let path = String::from(
                path.as_ref()
                    .as_os_str()
                    .to_str()
                    .ok_or(io::Error::from(io::ErrorKind::Other))?,
            );

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
            self.files.insert(path, entry);

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
                signature: Vpk2Signature::default(),
                files: HashMap::new(),
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
