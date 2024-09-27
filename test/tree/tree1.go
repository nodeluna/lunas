package tree

import (
	"math/rand"
	"os"
	"time"
)

var dirs_paths = [4]string{
	"running/one",
	"running/one/two",
	"running/one/two/three",
	"running/one/two/three/four",
}

var files_paths = [6]string{
	"running/one/file0",
	"running/one/two/file1",
	"running/one/two/file2",
	"running/one/two/three/file3",
	"running/one/two/three/file4",
	"running/one/two/three/four/file5",
}

func create_file(path string) error {
	file, err := os.Create(path)
	if err != nil {
		return err
	}
	defer file.Close()

	rand.Seed(time.Now().UnixNano())

	for i := 0; i < 1000; i++ {
		random_byte := byte(rand.Intn(256))

		_, err := file.Write([]byte{random_byte})
		if err != nil {
			return err
		}
	}

	return nil
}

func Make_tree1() error {
	for _, dir := range dirs_paths {
		err := os.Mkdir(dir, 0755)
		if err != nil {
			return err
		}
	}
	for _, file := range files_paths {
		err := create_file(file)
		if err != nil {
			return err
		}
	}

	return nil
}
