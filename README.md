# cpipe --- counting pipe

## Introduction
Did you ever want to know how fast your `tar` is or how
much data it has transferred already. How about using
`socket` or `nc` to copy files either with or
without compression over a fast network connection, which one is
faster?
      
If you want to know the answer, use `cpipe` as a totally
*unscientific* approach to measure throughput.  `Cpipe` copies its
standard input to its standard output while measuring the time it
takes to read an input buffer and write an output buffer. Statistics
of average throughput and the total amount of bytes copied are printed
to the standard error output.

## Example
The command
```bash
  tar cCf / - usr | <font color=red>cpipe</font> -vr -vw -vt > /dev/null
```
results in an output like
```terminal
...
  in:  19.541ms at    6.4MB/s (   4.7MB/s avg)    2.0MB
 out:   0.004ms at   30.5GB/s (  27.1GB/s avg)    2.0MB
thru:  19.865ms at    6.3MB/s (   4.6MB/s avg)    2.0MB
...
```

The first column shows the times it takes to handle one buffer of data
(128kB by default).  The read-call took 19.541ms, the write-call to
/dev/null took just 0.004ms and from the start of the read to the end
of write, it took 19.865ms.

The second column shows the result of dividing the buffer size
(128kB by default) by the times in the first column.


