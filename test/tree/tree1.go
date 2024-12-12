package tree

import (
	"fmt"
	"math/rand"
	"os"
	"path/filepath"
	"strconv"
	"time"
)

func create_file(path string) error {
	file, err := os.Create(path)
	if err != nil {
		return err
	}
	defer file.Close()

	rand.Seed(time.Now().UnixNano())

	const chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ"
	const numbers = "0123456789"
	random_data := chars
	length := rand.Intn(5000)
	for i := 0; i < length; i++ {
		random_data = random_data + chars + numbers + chars + numbers + chars + numbers
	}
	file.WriteString(random_data)

	return nil
}

func create_symlink(target string, link string) error {
	err := os.Symlink(target, link)
	if err != nil {
		return err
	}
	return nil
}

func Make_tree1(path string, depth int, dirs_width int) error {
	for dir_name := 0; dir_name < dirs_width; dir_name++ {
		new_dir := filepath.Join(path, strconv.Itoa(dir_name))
		err := os.Mkdir(new_dir, 0755)
		if err != nil {
			return err
		}
		fmt.Println("created dir: ", new_dir)

		for dir_depth := 1; dir_depth < depth; dir_depth++ {
			if dir_depth < depth {
				Make_tree1(new_dir, depth-1, dirs_width)
			}
		}

		rand.Seed(time.Now().UnixNano())
		files_num := rand.Intn(30)
		for i := dirs_width; i < files_num; i++ {
			filename := filepath.Join(new_dir, strconv.Itoa(i))
			err := create_file(filename)
			fmt.Println("created file: ", filename)
			if err != nil {
				return err
			}
			i += 1
			symlink_name := filepath.Join(new_dir, strconv.Itoa(i))
			err = create_symlink(filename, symlink_name)
			fmt.Println("created symlink: ", symlink_name)
			if err != nil {
				return err
			}
		}
	}

	return nil
}
