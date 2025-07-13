from pyodide.http import pyfetch
import asyncio
import datetime
import json
from js import boss_setjson, _log


args = ["###BOSS_ARGS###"]
BASE_URL = "http://112.218.47.138:8003"


async def get_studies() : 
    # 전체 study 목록과 각 study의 series 목록을 포함하는 데이터 반환
    result = {"studies" : []}

    try : 
        response = await pyfetch_get("data/studies")
        result["studies"] = response.get("studies", [])

        for study in result["studies"] : 
            study_datetime = study.get("StudyDateTime", "").split('T')
            study["StudyDate"] = datetime.datetime.strptime(study_datetime[0], '%Y-%m-%d').strftime('%Y-%m-%d')
            study["StudyTime"] = datetime.datetime.strptime(study_datetime[1], '%H:%M:%S').strftime('%H:%M:%S')
            
            study_uid = study.get("StudyKey")
            if not study_uid:
                continue

            series_response = await pyfetch_get(f"data/studies/{study_uid}/series")

            for series in series_response.get("series", []) :
                series["job"] = "ready"

            study["series"] = series_response.get("series", [])

    except Exception as e:
        _log("get_studies 에러: " + str(e))

    return result


async def pyfetch_get(query):
    # GET 요청 보내고 JSON 응답 반환
    try:
        response = await pyfetch(f"{BASE_URL}/{query}?")
        result = await response.json()
        return result

    except Exception as e:
        _log("요청 에러: " + str(e))
        return {}


async def main():
    result = await get_studies()
    json_result = json.dumps(result)
    boss_setjson("d.dicom." ,json_result)


asyncio.ensure_future(main())