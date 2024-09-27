package attributes

import (
	"fmt"
	"os"
	"syscall"
)

type Own struct {
	Uid uint32
	Gid uint32
}

func Get_lown(path string) (Own, error) {
	attrs, err := os.Lstat(path)
	if err != nil {
		return Own{}, err
	}
	stat, ok := attrs.Sys().(*syscall.Stat_t)
	if !ok {
		return Own{}, fmt.Errorf("error in syscall")
	}

	own := Own{Uid: stat.Uid, Gid: stat.Gid}
	return own, nil
}

func Get_own(path string) (Own, error) {
	attrs, err := os.Stat(path)
	if err != nil {
		return Own{}, err
	}
	stat, ok := attrs.Sys().(*syscall.Stat_t)
	if !ok {
		return Own{}, fmt.Errorf("error in syscall")
	}

	own := Own{Uid: stat.Uid, Gid: stat.Gid}
	return own, nil
}
