from pyodide.http import pyfetch
import asyncio
from js import boss_set, boss_get, boss_log, _log, alert



args = ["###BOSS_ARGS###"]


async def CheckDicomJobs(idx) : 
    try : 
        status_job = await boss_get(f'd.dcm.recent.{idx}.status_anls')
        
        if status_job == "" : 
            boss_set(f'd.dcm.recent.{idx}.status_anls', 'ready')

    except Exception as e: 
        _log(str(e))


async def main():
    await CheckDicomJobs(args[0])


asyncio.ensure_future(main())