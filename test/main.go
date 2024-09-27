package main

import (
	"fmt"
	"log"
	"os"
	"testing/cmd"
	"testing/tree"
	"testing/verify"
)

func main() {
	os.Mkdir("running", 0755)

	err := tree.Make_tree1()
	if err != nil {
		log.Fatal(err)
	}

	path1 := "running/one"
	path2 := "running/two"

	args := []string{"-p", path1, "-p", path2, "--attributes", "all=on", "-mkdir", "-P", "off", "-L", "off", "-B", "off"}
	err = cmd.Sync(args)
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
