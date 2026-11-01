// GPL-2.0-or-later. Run native tools with bounded waits and USB error detection.
using System;
using System.Diagnostics;
using System.IO;
using System.Threading;

public static class AvrProcessRunner
{
    public static int Run(string executable, string directory, string[] arguments)
    {
        using (var process = new Process())
        {
            process.StartInfo = new ProcessStartInfo(executable) {
                WorkingDirectory = directory,
                UseShellExecute = false,
                CreateNoWindow = true,
                RedirectStandardOutput = true,
                RedirectStandardError = true
            };
            foreach (string argument in arguments) process.StartInfo.ArgumentList.Add(argument);
            int transportError = 0;
            process.OutputDataReceived += (sender, e) => {
                if (e.Data != null) Console.WriteLine(e.Data);
            };
            process.ErrorDataReceived += (sender, e) => {
                if (e.Data == null) return;
                Console.Error.WriteLine(e.Data);
                if (e.Data.Contains("OS error:") ||
                    e.Data.Contains("cannot open USB device") ||
                    e.Data.Contains("cannot find USB device") ||
                    e.Data.Contains("wrong reading bytes") ||
                    e.Data.Contains("wrong count at writing") ||
                    e.Data.Contains("wrong response size"))
                    Interlocked.Exchange(ref transportError, 1);
            };
            var watch = Stopwatch.StartNew();
            process.Start();
            process.BeginOutputReadLine();
            process.BeginErrorReadLine();
            while (!process.WaitForExit(100))
            {
                if (Volatile.Read(ref transportError) != 0 || watch.Elapsed.TotalSeconds > 180)
                {
                    process.Kill();
                    process.WaitForExit();
                    throw new IOException("AVRDUDE was stopped after a USB transport error or timeout.");
                }
            }
            process.WaitForExit();
            if (Volatile.Read(ref transportError) != 0)
                throw new IOException("AVRDUDE reported a USB transport error.");
            return process.ExitCode;
        }
    }
}
