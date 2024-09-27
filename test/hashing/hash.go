package hashing

import (
	"crypto/sha1"
	"encoding/hex"
	"io"
	"os"
)

func File_hash(path string) (string, error) {
	file, err := os.Open(path)
	if err != nil {
		return "", err
	}
	defer file.Close()

	hash := sha1.New()
	if _, err := io.Copy(hash, file); err != nil {
		return "", err
	}

	checksum := hex.EncodeToString(hash.Sum(nil))

	return checksum, nil
}
