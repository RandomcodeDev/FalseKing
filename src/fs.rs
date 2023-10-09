use std::{
    ffi::OsString,
    fs::{FileType, Permissions},
    io,
    path::*,
    time::SystemTime,
};

/// std::fs::Metadata but transparent
#[allow(dead_code)]
#[derive(Clone, Debug)]
pub struct Metadata {
    file_type: FileType,
    is_dir: bool,
    is_file: bool,
    is_symlink: bool,
    len: u64,
    permissions: Permissions,
    modified: SystemTime,
    accessed: SystemTime,
    created: SystemTime,
}

#[allow(dead_code)]
impl Metadata {
    // This is for parity with std::fs
    #[allow(clippy::too_many_arguments)]
    pub fn new(
        file_type: FileType,
        is_dir: bool,
        is_file: bool,
        is_symlink: bool,
        len: u64,
        permissions: Permissions,
        modified: SystemTime,
        accessed: SystemTime,
        created: SystemTime,
    ) -> Self {
        Self {
            file_type,
            is_dir,
            is_file,
            is_symlink,
            len,
            permissions,
            modified,
            accessed,
            created,
        }
    }

    pub fn file_type(&self) -> FileType {
        self.file_type
    }

    pub fn is_dir(&self) -> bool {
        self.is_dir
    }
    pub fn is_file(&self) -> bool {
        self.is_file
    }
    pub fn is_symlink(&self) -> bool {
        self.is_symlink
    }
    pub fn len(&self) -> u64 {
        self.len
    }
    pub fn permissions(&self) -> Permissions {
        self.permissions.clone()
    }
    pub fn modified(&self) -> io::Result<SystemTime> {
        Ok(self.modified)
    }
    pub fn accessed(&self) -> io::Result<SystemTime> {
        Ok(self.accessed)
    }
    pub fn created(&self) -> io::Result<SystemTime> {
        Ok(self.created)
    }
}

impl From<std::fs::Metadata> for Metadata {
    fn from(value: std::fs::Metadata) -> Self {
        Self {
            file_type: value.file_type(),
            is_dir: value.is_dir(),
            is_file: value.is_file(),
            is_symlink: value.is_symlink(),
            len: value.len(),
            permissions: value.permissions(),
            modified: value.modified().unwrap(),
            accessed: value.accessed().unwrap(),
            created: value.created().unwrap(),
        }
    }
}

/// std::fs::DirEntry but transparent
#[derive(Debug)]
pub struct DirEntry(PathBuf, Metadata, FileType);

#[allow(dead_code)]
impl DirEntry {
    pub fn path(&self) -> PathBuf {
        self.0.clone()
    }

    pub fn metadata(&self) -> io::Result<Metadata> {
        Ok(self.1.clone())
    }

    pub fn file_type(&self) -> io::Result<FileType> {
        Ok(self.2)
    }

    pub fn file_name(&self) -> OsString {
        self.0.file_name().unwrap().into()
    }
}

impl From<std::fs::DirEntry> for DirEntry {
    fn from(value: std::fs::DirEntry) -> Self {
        Self(
            value.path(),
            value.metadata().ok().unwrap().into(),
            value.file_type().ok().unwrap(),
        )
    }
}

/// std::fs but you can make other things accessible with it (like a VPK)
pub trait FileSystem {
    /// Check if a file exists
    fn try_exists<P: AsRef<Path>>(&self, path: P) -> io::Result<bool>;

    /// Canonicalize a path
    fn canonicalize<P: AsRef<Path>>(&self, path: P) -> io::Result<PathBuf>;

    /// Copy a file
    fn copy<P: AsRef<Path>, Q: AsRef<Path>>(&self, from: P, to: Q) -> io::Result<u64>;

    /// Create a directory
    fn create_dir<P: AsRef<Path>>(&self, path: P) -> io::Result<()>;

    /// Create a directory recursively
    fn create_dir_all<P: AsRef<Path>>(&self, path: P) -> io::Result<()>;

    /// Create a hard link
    fn hard_link<P: AsRef<Path>, Q: AsRef<Path>>(&self, original: P, link: Q) -> io::Result<()>;

    /// Get the metadata of a path
    fn metadata<P: AsRef<Path>>(&self, path: P) -> io::Result<Metadata>;

    /// Read a file into a vector
    fn read<P: AsRef<Path>>(&self, path: P) -> io::Result<Vec<u8>>;

    /// Directory iterator
    fn read_dir<P: AsRef<Path>>(
        &self,
        path: P,
    ) -> io::Result<Box<dyn Iterator<Item = io::Result<DirEntry>>>>;

    /// Get the path a symlink points to
    fn read_link<P: AsRef<Path>>(&self, path: P) -> io::Result<PathBuf>;

    /// Read a file into a string
    fn read_to_string<P: AsRef<Path>>(&self, path: P) -> io::Result<String>;

    /// Remove an empty directory
    fn remove_dir<P: AsRef<Path>>(&self, path: P) -> io::Result<()>;

    /// Annihilate a directory tree
    fn remove_dir_all<P: AsRef<Path>>(&self, path: P) -> io::Result<()>;

    /// Remove a file
    fn remove_file<P: AsRef<Path>>(&self, path: P) -> io::Result<()>;

    /// Rename a file
    fn rename<P: AsRef<Path>, Q: AsRef<Path>>(&self, to: P, from: Q) -> io::Result<()>;

    /// Change permissions
    fn set_permissions<P: AsRef<Path>>(&self, path: P, permissions: Permissions) -> io::Result<()>;

    /// Create a symlink (not deprecated)
    fn soft_link<P: AsRef<Path>, Q: AsRef<Path>>(&self, from: P, to: Q) -> io::Result<()>;

    /// Get metadata of a file, not following symlinks
    fn symlink_metadata<P: AsRef<Path>>(&self, path: P) -> io::Result<Metadata>;

    /// Write a slice to a file
    fn write<P: AsRef<Path>, C: AsRef<[u8]>>(&self, path: P, contents: C) -> io::Result<()>;
}

/// FileSystem using std::fs
pub struct StdFileSystem(());

impl StdFileSystem {
    #[allow(dead_code)]
    pub fn new() -> Self {
        Self(())
    }
}

impl FileSystem for StdFileSystem {
    fn try_exists<P: AsRef<Path>>(&self, path: P) -> io::Result<bool> {
        std::fs::try_exists(path)
    }

    fn canonicalize<P: AsRef<Path>>(&self, path: P) -> io::Result<PathBuf> {
        std::fs::canonicalize(path)
    }

    fn copy<P: AsRef<Path>, Q: AsRef<Path>>(&self, from: P, to: Q) -> io::Result<u64> {
        std::fs::copy(from, to)
    }

    fn create_dir<P: AsRef<Path>>(&self, path: P) -> io::Result<()> {
        std::fs::create_dir(path)
    }

    fn create_dir_all<P: AsRef<Path>>(&self, path: P) -> io::Result<()> {
        std::fs::create_dir_all(path)
    }

    fn hard_link<P: AsRef<Path>, Q: AsRef<Path>>(&self, original: P, link: Q) -> io::Result<()> {
        std::fs::hard_link(original, link)
    }

    fn metadata<P: AsRef<Path>>(&self, path: P) -> io::Result<Metadata> {
        match std::fs::metadata(path) {
            Ok(path) => Ok(path.into()),
            Err(err) => Err(err),
        }
    }

    fn read<P: AsRef<Path>>(&self, path: P) -> io::Result<Vec<u8>> {
        std::fs::read(path)
    }

    fn read_dir<P: AsRef<Path>>(
        &self,
        _path: P,
    ) -> io::Result<Box<dyn Iterator<Item = io::Result<DirEntry>>>> {
        todo!()
    }

    fn read_link<P: AsRef<Path>>(&self, path: P) -> io::Result<PathBuf> {
        std::fs::read_link(path)
    }

    fn read_to_string<P: AsRef<Path>>(&self, path: P) -> io::Result<String> {
        std::fs::read_to_string(path)
    }

    fn remove_dir<P: AsRef<Path>>(&self, path: P) -> io::Result<()> {
        std::fs::remove_dir(path)
    }

    fn remove_dir_all<P: AsRef<Path>>(&self, path: P) -> io::Result<()> {
        std::fs::remove_dir_all(path)
    }

    fn remove_file<P: AsRef<Path>>(&self, path: P) -> io::Result<()> {
        std::fs::remove_file(path)
    }

    fn rename<P: AsRef<Path>, Q: AsRef<Path>>(&self, to: P, from: Q) -> io::Result<()> {
        std::fs::rename(from, to)
    }

    fn set_permissions<P: AsRef<Path>>(&self, path: P, perm: Permissions) -> io::Result<()> {
        std::fs::set_permissions(path, perm)
    }

    fn soft_link<P: AsRef<Path>, Q: AsRef<Path>>(&self, from: P, to: Q) -> io::Result<()> {
        #[cfg(windows)]
        match self.metadata(&from) {
            Ok(metadata) => {
                if metadata.is_dir {
                    std::os::windows::fs::symlink_dir(from, to)
                } else {
                    std::os::windows::fs::symlink_file(from, to)
                }
            }
            Err(err) => Err(err),
        }
        #[cfg(unix)]
        std::os::unix::fs::symlink(from, to)
    }

    fn symlink_metadata<P: AsRef<Path>>(&self, path: P) -> io::Result<Metadata> {
        match std::fs::symlink_metadata(path) {
            Ok(path) => Ok(path.into()),
            Err(err) => Err(err),
        }
    }

    fn write<P: AsRef<Path>, C: AsRef<[u8]>>(&self, path: P, contents: C) -> io::Result<()> {
        std::fs::write(path, contents)
    }
}
