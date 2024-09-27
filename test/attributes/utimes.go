package attributes

import (
	"fmt"
	"os"
	"syscall"
	"time"
)

type Utimes struct {
	Atime time.Time
	Mtime time.Time
}

func Get_lutimes(path string) (Utimes, error) {
	attrs, err := os.Lstat(path)
	if err != nil {
		return Utimes{}, err
	}
	stat, ok := attrs.Sys().(*syscall.Stat_t)
	if !ok {
		return Utimes{}, fmt.Errorf("error in syscall")
	}
	atime := time.Unix(int64(stat.Atim.Sec), 0)
	mtime := attrs.ModTime().Truncate(time.Second)

	utimes := Utimes{Atime: atime, Mtime: mtime}
	return utimes, nil
}

func Get_utimes(path string) (Utimes, error) {
	attrs, err := os.Stat(path)
	if err != nil {
		return Utimes{}, err
	}
	stat, ok := attrs.Sys().(*syscall.Stat_t)
	if !ok {
		return Utimes{}, fmt.Errorf("error in syscall")
	}
	atime := time.Unix(int64(stat.Atim.Sec), 0)
	mtime := attrs.ModTime().Truncate(time.Second)

	utimes := Utimes{Atime: atime, Mtime: mtime}
	return utimes, nil
}
