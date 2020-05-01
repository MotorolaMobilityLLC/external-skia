// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package main

import (
	"bufio"
	"flag"
	"math/rand"
	"runtime"
	"strings"
	"sync"
	"sync/atomic"

	"go.skia.org/infra/go/exec"
	"go.skia.org/infra/go/util"
	"go.skia.org/infra/task_driver/go/td"
)

type work struct {
	Sources []string
	Flags   []string
}

func main() {
	var (
		projectId = flag.String("project_id", "", "ID of the Google Cloud project.")
		taskId    = flag.String("task_id", "", "ID of this task.")
		taskName  = flag.String("task_name", "", "Name of the task.")
		local     = flag.Bool("local", true, "True if running locally (as opposed to on the bots)")
		output    = flag.String("o", "", "If provided, dump a JSON blob of step data to the given file. Prints to stdout if '-' is given.")

		resources = flag.String("resources", "resources", "Passed to fm -i.")
	)
	ctx := td.StartRun(projectId, taskId, taskName, output, local)
	defer td.EndRun(ctx)

	if flag.NArg() < 1 {
		td.Fatalf(ctx, "Please pass an fm binary.")
	}
	fm := flag.Arg(0)

	// Run fm --flag to find the names of all linked GMs or tests.
	query := func(flag string) []string {
		stdout, err := exec.RunCwd(ctx, ".", fm, flag, "-i", *resources)
		if err != nil {
			td.Fatal(ctx, err)
		}

		lines := []string{}
		scanner := bufio.NewScanner(strings.NewReader(stdout))
		for scanner.Scan() {
			lines = append(lines, scanner.Text())
		}
		if err := scanner.Err(); err != nil {
			td.Fatal(ctx, err)
		}
		return lines
	}
	gms := query("--listGMs")
	tests := query("--listTests")

	parse := func(job []string) *work {
		w := &work{}

		for _, token := range job {
			// Everything after # is a comment.
			if strings.HasPrefix(token, "#") {
				break
			}

			// Treat "gm" or "gms" as a shortcut for all known GMs.
			if token == "gm" || token == "gms" {
				w.Sources = append(w.Sources, gms...)
				continue
			}
			// Same for tests.
			if token == "test" || token == "tests" {
				w.Sources = append(w.Sources, tests...)
				continue
			}

			// Is this a flag to pass through to FM?
			if parts := strings.Split(token, "="); len(parts) == 2 {
				f := "-"
				if len(parts[0]) > 1 {
					f += "-"
				}
				f += parts[0]

				w.Flags = append(w.Flags, f, parts[1])
				continue
			}

			// Anything else must be the name of a source for FM to run.
			w.Sources = append(w.Sources, token)
		}

		return w
	}

	// TODO: this doesn't have to be hard coded, of course.
	// TODO: add some .skps or images to demo that.
	script := `
	b=cpu tests
	b=cpu gms
	b=cpu gms skvm=true

	#b=cpu gms skvm=true gamut=p3
	#b=cpu gms skvm=true ct=565
	`
	jobs := [][]string{}
	scanner := bufio.NewScanner(strings.NewReader(script))
	for scanner.Scan() {
		jobs = append(jobs, strings.Fields(scanner.Text()))
	}
	if err := scanner.Err(); err != nil {
		td.Fatal(ctx, err)
	}

	var failures int32 = 0
	wg := &sync.WaitGroup{}

	worker := func(queue chan work) {
		for w := range queue {
			cmd := []string{}
			cmd = append(cmd, fm)
			cmd = append(cmd, "-i", *resources)
			cmd = append(cmd, w.Flags...)
			cmd = append(cmd, "-s")
			cmd = append(cmd, w.Sources...)

			if _, err := exec.RunCwd(ctx, ".", cmd...); err != nil {
				if len(w.Sources) == 1 {
					// If a source ran alone and failed, that's just a failure.
					atomic.AddInt32(&failures, 1)
					td.FailStep(ctx, err)
				} else {
					// If a batch of sources ran and failed, split them up and try again.
					for _, source := range w.Sources {
						wg.Add(1)
						queue <- work{[]string{source}, w.Flags}
					}
				}
			}
			wg.Done()
		}
	}

	workers := runtime.NumCPU()
	queue := make(chan work, 1<<20)
	for i := 0; i < workers; i++ {
		go worker(queue)
	}

	for _, job := range jobs {
		w := parse(job)
		if len(w.Sources) == 0 {
			continue
		}

		// Shuffle the sources randomly as a cheap way to approximate evenly expensive batches.
		rand.Shuffle(len(w.Sources), func(i, j int) {
			w.Sources[i], w.Sources[j] = w.Sources[j], w.Sources[i]
		})

		// Round up so there's at least one source per batch.
		batch := (len(w.Sources) + workers - 1) / workers
		util.ChunkIter(len(w.Sources), batch, func(start, end int) error {
			wg.Add(1)
			queue <- work{w.Sources[start:end], w.Flags}
			return nil
		})
	}
	wg.Wait()

	if failures > 0 {
		td.Fatalf(ctx, "%v runs of %v failed after retries.", failures, fm)
	}
}
