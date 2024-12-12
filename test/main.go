package main

import (
	"fmt"
	"log"
	"os"
	"path/filepath"
	"testing/cmd"
	"testing/tree"
	"testing/verify"
	"time"
)

func running() {
	path1 := "running/one"
	path2 := "running/two"

	args := []string{"-p", path1, "-p", path2, "-q", "--attributes", "all=on", "-mkdir", "-P", "off", "-L", "off", "-B", "off"}
	err := cmd.Sync(args)
	if err != nil {
		fmt.Println(err)
	}

	input_paths := []verify.Input_path{
		{Name: path1, Srcdest: verify.SRCDEST, Remote: false},
		{Name: path2, Srcdest: verify.SRCDEST, Remote: false},
	}

	err0 := verify.Verify(input_paths)
	if err0 != nil {
		log.Fatal(err0)
	}

}

func exists(path string) bool {
	_, err := os.Lstat(path)
	return err == nil || !os.IsNotExist(err)
}

func main() {
	if exists("running") == false {
		os.Mkdir("running", 0755)
	}

	if exists("running/one") == false {
		cwd, _ := os.Getwd()
		dir := filepath.Join(cwd, "running/one")
		os.Mkdir(dir, 0755)
		err := tree.Make_tree1(dir, 3, 3)
		if err != nil {
			log.Fatal(err)
		}
	}

	counter := 1
	for {
		running()
		os.RemoveAll("running/two")
		fmt.Println("removed 'running/two'")
		fmt.Println("sleeping for 3 seconds")
		fmt.Println("-> running counter: ", counter)
		time.Sleep(3 * time.Second)
		counter += 1
	}

}
