package verify

import (
	"fmt"
	"io/fs"
	"os"
	"path/filepath"
	"strings"
	"testing/attributes"
	"testing/hashing"
)

type Input_path struct {
	Name    string
	Srcdest int16
	Remote  bool
}

var SRC int16 = 1
var DEST int16 = 2
var SRCDEST int16 = 3

func file_checks(src string, dest string) error {
	utimes_src, err := attributes.Get_lutimes(src)
	if err != nil {
		return err
	}
	utimes_dest, err := attributes.Get_lutimes(dest)
	if err != nil {
		return err
	}

	if utimes_src.Atime != utimes_dest.Atime || utimes_src.Mtime != utimes_dest.Mtime {
		return fmt.Errorf("utimes are different\n", "src: ", src, "\ndest: ", dest)
	}

	own_src, err := attributes.Get_lown(src)
	if err != nil {
		return err
	}
	own_dest, err := attributes.Get_lown(dest)
	if err != nil {
		return err
	}

	if own_src.Uid != own_dest.Uid || own_src.Gid != own_dest.Gid {
		return fmt.Errorf("Uid/Gid are different\n", "src: ", src, "\ndest: ", dest)
	}

	checksum_src, err := hashing.File_hash(src)
	if err != nil {
		return err
	}
	checksum_dest, err := hashing.File_hash(dest)
	if err != nil {
		return err
	}
	if checksum_src != checksum_dest {
		return fmt.Errorf("checksums don't match\n", "src: ", src, "\ndest: ", dest)
	}

	return nil
}

func Verify(input_paths []Input_path) error {
	fmt.Println("[verifying files...]")

	for _, input_path_src := range input_paths {
		if input_path_src.Srcdest != SRC && input_path_src.Srcdest != SRCDEST {
			continue
		}
		for _, input_path_dest := range input_paths {
			if input_path_dest.Name == input_path_src.Name {
				continue
			} else if input_path_dest.Srcdest != DEST && input_path_dest.Srcdest != SRCDEST {
				continue
			}

			filepath.WalkDir(input_path_src.Name, func(path string, info fs.DirEntry, err error) error {
				if err != nil {
					return err
				}
				dest_path := strings.ReplaceAll(path, input_path_src.Name, input_path_dest.Name)

				info_dest, err0 := os.Lstat(dest_path)
				if os.IsNotExist(err0) || err0 != nil {
					return err0
				}

				if info.Type() != info_dest.Mode().Type() {
					return fmt.Errorf("types dont match\n", "src: ", path, "\ndest:", dest_path)
				}
				if info.IsDir() {
					return nil
				}

				err1 := file_checks(path, dest_path)
				if err1 != nil {
					return err1
				}

				return nil
			})
		}
	}

	fmt.Println("[verification done successfully]")
	return nil
}
