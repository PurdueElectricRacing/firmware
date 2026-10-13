pub fn find_path<F>(
    relative_path: impl AsRef<std::path::Path>,
    matches: F,
) -> Option<std::path::PathBuf>
where
    F: Fn(&std::path::Path) -> bool,
{
    let current_directory = std::env::current_dir().ok()?;

    // Resource paths are launch-directory-relative; run DaqApp from its package directory.
    let candidate = current_directory.join(relative_path);
    matches(&candidate).then_some(candidate)
}

pub fn find_file(relative_path: impl AsRef<std::path::Path>) -> Option<std::path::PathBuf> {
    find_path(relative_path, std::path::Path::is_file)
}

pub fn read_file(relative_path: impl AsRef<std::path::Path>) -> Option<String> {
    find_file(relative_path).and_then(|path| std::fs::read_to_string(path).ok())
}
