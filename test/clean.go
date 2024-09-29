package main

import (
	"log"
	"os"
	"path/filepath"
)

func main() {
	running_path := "./running"
	files, err := os.ReadDir(running_path)
	if os.IsNotExist(err) {
		return
	} else if err != nil {
		log.Fatal(err)
	}

	for _, file := range files {
		file_path := filepath.Join(running_path, file.Name())
		err := os.RemoveAll(file_path)
		if err != nil {
			log.Fatal(err)
		}
	}

	os.Remove("running")

	log.Println("directory ./running emptied successfully")
}
