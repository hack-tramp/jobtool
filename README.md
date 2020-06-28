# jobtool
This console app lets you create .JOB task scheduler files, used in older versions of windows<br>
<br>
ms docs here:
https://docs.microsoft.com/en-us/openspecs/windows_protocols/ms-tsch/8a585672-81b8-4b3a-912a-310b01da3518
<br>some extra info here:
https://web.archive.org/web/20190629040200/https://forensicswiki.org/wiki/Windows_Job_File_Format
<br>
<br>
Usage example:<br><br>
`
jobtool sometask.job 60 10 32 10 C:\WINDOWS\system32\cmd.exe C:\WINDOWS\system32 Administrator 2020 7 25 14 30
`
<br>
<br>
The above example will execute cmd at 14.30 on 25 july 2020 - task author Admin
<br>
<b>Params</b> (more info in official ms docs):
- filename for JOB file
- idle deadline (maximum time in minutes to wait<br> for the machine to become idle for Idle Wait minutes.)<br>
- idle wait (value in minutes. The machine remains <br> idle for this many minutes before it runs the task.)
- priority
- maximum run time (number of milliseconds the server<br> will wait for the task to complete)
- app name to execute (inc. path)
- working directory
- task author
- begin year
- begin month (1-12)
- begin day 
- begin hour
- begin minute

