from pyodide.http import pyfetch
import urllib
import asyncio
import json
from js import alert, _log, boss_set, boss_setjson
import state


args = ["###BOSS_ARGS###"]
BASE_URL = "http://112.218.47.138:8003"

user_info = None


async def get_access_token(username, password):
    # Keycloak에 access_token 요청
    url = f"{BASE_URL}/account/token"
    data = {
        "username": username,
        "password": password
    }

    encoded_data = urllib.parse.urlencode(data)

    try:
        response = await pyfetch(
            url,
            method="POST",
            body=encoded_data,
            headers={
                "Content-Type": "application/x-www-form-urlencoded"
            }
        )

        if response.status != 200:
            error_json = await response.json()
            _log(error_json.get("error_description", "Token request failed"))

        return await response.json()

    except Exception as e:
        alert("Login Fail : " + str(e))


async def get_user_info(access_token):
    # 토큰을 사용해 사용자 정보 요청
    url = f"{BASE_URL}/users/me"
    try:
        response = await pyfetch(
            url,
            method="GET",
            headers={
                "Authorization": f"Bearer {access_token}"
            }
        )

        if response.status != 200:
            _log(f"사용자 정보 요청 실패 (status {response.status})")
            return None

        return await response.json()

    except Exception as e:
        alert("get_user_info 에러: " + str(e))
        return None


async def login_to_keycloak(username, password):
    # 전체 로그인 흐름 관리 - access token 받아온 후, 사용자 정보 요청 -> data 페이지 이동
    try:
        global user_info

        token_data = await get_access_token(username, password)
        token = token_data.get("access_token", "")

        if not token:
            alert("Login failed: 토큰이 비어있습니다.")
            return

        state.token = token

        user_info = await get_user_info(state.token)
        json_user_info = json.dumps(user_info)

        if user_info is not None:
            boss_setjson('d.user.me.', json_user_info)
        else:
            _log("사용자 정보를 불러올 수 없습니다.")
        
        boss_set('d.data.page', 'data')

    except Exception as e:
        alert("로그인 실패: " + str(e))
        _log("login_to_keycloak 에러: " + str(e))


async def main():
    id = args[0]
    pw = args[1]
    await login_to_keycloak(id, pw)

    boss_set('d.server_url', f"'{BASE_URL}'")


asyncio.ensure_future(main())