use crate::protocol::{self, Mapping, Profiles};
use serde::{Deserialize, Serialize};
use std::fs::{self, File, OpenOptions};
use std::io::{Read, Write};
use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicU64, Ordering};

const MAX_SIZE: u64 = 64 * 1024;

#[derive(Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
struct Document {
    version: u8,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    mode: Option<String>,
    #[serde(default, skip_serializing_if = "Option::is_none")]
    divider: Option<String>,
    keymap: Vec<u8>,
}

#[derive(Deserialize, Serialize)]
#[serde(deny_unknown_fields)]
struct ProfileDocument {
    version: u8,
    mode: String,
    divider: String,
    banks: Vec<Vec<u8>>,
    shared: Vec<u8>,
}

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
pub enum Imported {
    All(Profiles),
    Active(Mapping),
}

fn decode_profiles(text: &str) -> Result<Imported, String> {
    let value: toml::Value = toml::from_str(text).map_err(|e| e.to_string())?;
    if value.get("version").and_then(toml::Value::as_integer) != Some(3) {
        return decode(text).map(Imported::Active);
    }
    let doc: ProfileDocument = toml::from_str(text).map_err(|e| e.to_string())?;
    let banks = doc
        .banks
        .into_iter()
        .map(|bank| bank.try_into().map_err(|_| "profile must contain 32 pads"))
        .collect::<Result<Vec<[u8; 32]>, _>>()?
        .try_into()
        .map_err(|_| "exactly six profiles required")?;
    let profiles = Profiles {
        banks,
        shared: doc
            .shared
            .try_into()
            .map_err(|_| "exactly eight shared usages required")?,
        mode: MODES
            .iter()
            .position(|&s| s == doc.mode)
            .ok_or("invalid mode")? as u8,
        divider: DIVIDERS
            .iter()
            .position(|&s| s == doc.divider)
            .ok_or("invalid divider")? as u8,
    }
    .validate()?;
    Ok(Imported::All(profiles))
}

fn encode_profiles(profiles: Profiles) -> Result<String, String> {
    profiles.validate()?;
    toml::to_string(&ProfileDocument {
        version: 3,
        mode: MODES[usize::from(profiles.mode)].into(),
        divider: DIVIDERS[usize::from(profiles.divider)].into(),
        banks: profiles.banks.iter().map(|bank| bank.to_vec()).collect(),
        shared: profiles.shared.to_vec(),
    })
    .map_err(|e| e.to_string())
}

const MODES: [&str; 6] = ["32k", "16k", "9k", "8k", "4k", "2k"];
const DIVIDERS: [&str; 7] = ["none", "single", "all", "2k", "4k", "8k", "9k"];

fn decode(text: &str) -> Result<Mapping, String> {
    let document: Document = toml::from_str(text).map_err(|e| e.to_string())?;
    let (mode, divider) = match document.version {
        1 if document.mode.is_none() && document.divider.is_none() => (0, 4),
        2 => {
            let mode = MODES
                .iter()
                .position(|&s| Some(s) == document.mode.as_deref())
                .ok_or("missing or invalid keyboard mode")?;
            let divider = DIVIDERS
                .iter()
                .position(|&s| Some(s) == document.divider.as_deref())
                .ok_or("missing or invalid divider")?;
            (mode as u8, divider as u8)
        }
        _ => return Err("unsupported or malformed keymap version".into()),
    };
    Mapping {
        keys: protocol::keymap(&document.keymap)?,
        mode,
        divider,
    }
    .validate()
}

#[cfg(test)]
fn encode(map: Mapping) -> Result<String, String> {
    map.validate()?;
    toml::to_string(&Document {
        version: 2,
        mode: Some(MODES[usize::from(map.mode)].into()),
        divider: Some(DIVIDERS[usize::from(map.divider)].into()),
        keymap: map.keys.to_vec(),
    })
    .map_err(|e| e.to_string())
}

fn read(path: &Path) -> Result<Imported, String> {
    let mut bytes = Vec::new();
    File::open(path)
        .and_then(|file| file.take(MAX_SIZE + 1).read_to_end(&mut bytes))
        .map_err(|e| format!("cannot read {}: {e}", path.display()))?;
    if bytes.len() as u64 > MAX_SIZE {
        return Err("keymap file exceeds 64 KB".into());
    }
    decode_profiles(std::str::from_utf8(&bytes).map_err(|e| e.to_string())?)
}

fn write(path: &Path, profiles: Profiles) -> Result<(), String> {
    let text = encode_profiles(profiles)?;
    static NEXT: AtomicU64 = AtomicU64::new(0);
    let (temp, mut file) = loop {
        let temp = path.with_file_name(format!(
            ".tasoller-{}-{}.tmp",
            std::process::id(),
            NEXT.fetch_add(1, Ordering::Relaxed)
        ));
        match OpenOptions::new().write(true).create_new(true).open(&temp) {
            Ok(file) => break (temp, file),
            Err(e) if e.kind() == std::io::ErrorKind::AlreadyExists => continue,
            Err(e) => return Err(format!("cannot export {}: {e}", path.display())),
        }
    };
    let result = (|| {
        file.write_all(text.as_bytes())?;
        file.sync_all()?;
        drop(file);
        replace(&temp, path)
    })();
    if result.is_err() {
        let _ = fs::remove_file(&temp);
    }
    result.map_err(|e| format!("cannot export {}: {e}", path.display()))
}

#[cfg(windows)]
fn replace(from: &Path, to: &Path) -> std::io::Result<()> {
    use std::os::windows::ffi::OsStrExt;
    use windows_sys::Win32::Storage::FileSystem::{
        MOVEFILE_REPLACE_EXISTING, MOVEFILE_WRITE_THROUGH, MoveFileExW,
    };
    let from: Vec<_> = from.as_os_str().encode_wide().chain(Some(0)).collect();
    let to: Vec<_> = to.as_os_str().encode_wide().chain(Some(0)).collect();
    if unsafe {
        MoveFileExW(
            from.as_ptr(),
            to.as_ptr(),
            MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH,
        )
    } == 0
    {
        return Err(std::io::Error::last_os_error());
    }
    Ok(())
}

#[cfg(not(windows))]
fn replace(from: &Path, to: &Path) -> std::io::Result<()> {
    fs::rename(from, to)
}

#[cfg(windows)]
fn dialog(save: bool) -> Result<Option<PathBuf>, String> {
    use std::ffi::OsString;
    use std::os::windows::ffi::OsStringExt;
    use windows_sys::Win32::UI::Controls::Dialogs::*;
    let mut path = vec![0u16; 32768];
    let filter: Vec<_> = "Keymap TOML (*.toml)\0*.toml\0\0".encode_utf16().collect();
    let extension: Vec<_> = "toml\0".encode_utf16().collect();
    let title: Vec<_> = if save {
        "Export keymap\0"
    } else {
        "Import keymap\0"
    }
    .encode_utf16()
    .collect();
    let mut options = OPENFILENAMEW {
        lStructSize: std::mem::size_of::<OPENFILENAMEW>() as u32,
        lpstrFilter: filter.as_ptr(),
        nFilterIndex: 1,
        lpstrFile: path.as_mut_ptr(),
        nMaxFile: path.len() as u32,
        lpstrTitle: title.as_ptr(),
        lpstrDefExt: extension.as_ptr(),
        Flags: OFN_EXPLORER
            | OFN_NOCHANGEDIR
            | OFN_PATHMUSTEXIST
            | if save {
                OFN_OVERWRITEPROMPT
            } else {
                OFN_FILEMUSTEXIST
            },
        ..Default::default()
    };
    let accepted = unsafe {
        if save {
            GetSaveFileNameW(&mut options)
        } else {
            GetOpenFileNameW(&mut options)
        }
    };
    if accepted == 0 {
        let error = unsafe { CommDlgExtendedError() };
        return if error == 0 {
            Ok(None)
        } else {
            Err(format!("file dialog failed: 0x{error:04X}"))
        };
    }
    let end = path
        .iter()
        .position(|&c| c == 0)
        .ok_or("invalid dialog path")?;
    Ok(Some(PathBuf::from(OsString::from_wide(&path[..end]))))
}

#[cfg(not(windows))]
fn dialog(_: bool) -> Result<Option<PathBuf>, String> {
    Err("keymap file dialogs require Windows".into())
}

pub fn export(profiles: Profiles) -> Result<bool, String> {
    profiles.validate()?;
    let Some(path) = dialog(true)? else {
        return Ok(false);
    };
    write(&path, profiles)?;
    Ok(true)
}

pub fn import() -> Result<Option<Imported>, String> {
    dialog(false)?.map(|path| read(&path)).transpose()
}

#[cfg(test)]
mod tests {
    use super::*;

    fn mapping(keys: [u8; 40]) -> Mapping {
        Mapping {
            keys,
            mode: 0,
            divider: 4,
        }
    }

    fn profiles(usage: u8) -> Profiles {
        Profiles {
            banks: [[usage; 32]; 6],
            shared: [usage; 8],
            mode: 4,
            divider: 4,
        }
    }

    #[test]
    fn profile_schema_validation_and_legacy_scope() {
        let mut p = profiles(4);
        p.banks[4] = std::array::from_fn(|pad| [4, 7, 11, 12][(31 - pad) / 8]);
        let text = encode_profiles(p).unwrap();
        assert_eq!(decode_profiles(&text).unwrap(), Imported::All(p));
        let legacy = mapping([4; 40]);
        assert_eq!(
            decode_profiles(&encode(legacy).unwrap()).unwrap(),
            Imported::Active(legacy)
        );
        assert_eq!(
            decode_profiles(&format!("version = 1\nkeymap = {:?}", legacy.keys)).unwrap(),
            Imported::Active(legacy)
        );
        for invalid in [
            text.replace("version = 3", "version = 4"),
            text.replace("mode = \"4k\"", "mode = \"7k\""),
            format!("{text}\nunknown = 1"),
        ] {
            assert!(decode_profiles(&invalid).is_err());
        }
        p.banks[4][0] = 0xe0;
        assert!(encode_profiles(p).is_err());
        p.banks[4][0] = 5;
        assert!(encode_profiles(p).is_err());
    }

    #[test]
    fn schema_and_roundtrip() {
        let map = mapping(std::array::from_fn(
            |i| if i == 0 { 0 } else { i as u8 + 3 },
        ));
        let text = encode(map).unwrap();
        assert_eq!(decode(&text).unwrap(), map);
        for invalid in [
            text.replace("version = 2", "version = 3"),
            text.replace("32k", "7k"),
            text.replace("4k", "3k"),
            text.replace("mode = \"32k\"", ""),
            text.replace("divider = \"4k\"", ""),
            text.replace("version = 2", "version = 1"),
            text.replace("32k", "4k"),
            format!("{text}\nunknown = 1"),
            "version = 1\nkeymap = []".into(),
            "version = 1\nkeymap = [".into(),
            "keymap = []".into(),
        ] {
            assert!(decode(&invalid).is_err());
        }
        for usage in [1, 2, 3, 0xa5, 255, 256, -1] {
            let entries = vec![usage.to_string(); 40].join(",");
            assert!(decode(&format!("version = 1\nkeymap = [{entries}]")).is_err());
        }
        for length in [39, 41] {
            assert!(
                decode(&format!(
                    "version = 1\nkeymap = [{}]",
                    vec!["4"; length].join(",")
                ))
                .is_err()
            );
        }
        assert_eq!(
            decode(&encode(mapping([0xa4; 40])).unwrap()).unwrap(),
            mapping([0xa4; 40])
        );
        let legacy = format!("version = 1\nkeymap = {:?}", map.keys);
        assert_eq!(decode(&legacy).unwrap(), map);
        for mode in 0..6 {
            for divider in 0..7 {
                let mut grouped = Mapping {
                    mode,
                    divider,
                    ..map
                };
                grouped.normalize();
                assert_eq!(decode(&encode(grouped).unwrap()).unwrap(), grouped);
            }
        }
    }

    #[test]
    fn file_errors_and_atomic_replacement() {
        let dir = std::env::temp_dir().join(format!(
            "tasoller-test-{}-{}",
            std::process::id(),
            std::time::SystemTime::now()
                .duration_since(std::time::UNIX_EPOCH)
                .unwrap()
                .as_nanos()
        ));
        fs::create_dir(&dir).unwrap();
        let path = dir.join("keymap.toml");
        assert!(read(&path).is_err());
        write(&path, profiles(4)).unwrap();
        assert!(write(&path, profiles(3)).is_err());
        assert_eq!(read(&path).unwrap(), Imported::All(profiles(4)));
        write(&path, profiles(0)).unwrap();
        assert_eq!(read(&path).unwrap(), Imported::All(profiles(0)));
        assert!(write(&dir, profiles(4)).is_err());
        assert_eq!(fs::read_dir(&dir).unwrap().count(), 1);
        fs::write(&path, vec![b' '; MAX_SIZE as usize + 1]).unwrap();
        assert!(read(&path).unwrap_err().contains("64 KB"));
        fs::write(&path, [0xff]).unwrap();
        assert!(read(&path).is_err());
        fs::remove_dir_all(dir).unwrap();
    }
}
