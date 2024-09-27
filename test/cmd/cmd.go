package cmd

import (
	"fmt"
	"os/exec"
)

func Sync(args []string) error {
	fmt.Println("[syncing...]")
	stdout, err := exec.Command("lunas", args...).Output()
	if err != nil {
		return err
	}

	fmt.Println(string(stdout))
	return nil
}
