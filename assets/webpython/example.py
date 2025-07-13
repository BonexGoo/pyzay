from js import (
    alert, JSON,
    boss_findfile, boss_pickupfile, boss_infofile,
    boss_readfile, boss_writefile,
    boss_resetfile, boss_movefile, boss_hashfile)
args = ["###BOSS_ARGS###"]

async def test1(path):
    files = await boss_findfile(path)
    alert(files)

async def test2():
    filepath = await boss_pickupfile("png", "image/img1")
    alert(filepath)

async def test3(path):
    fileinfo = await boss_infofile(path)
    alert(JSON.stringify(fileinfo))

async def test4():
    base64 = await boss_readfile("aaa/login.py")
    alert(base64)

async def test5():
    filesize = await boss_writefile("bbb/1.txt", "aGVsbG8hISE=")
    alert(filesize)

async def test6():
    success = await boss_resetfile("bbb/1.txt")
    alert(success)

async def test7():
    success = await boss_movefile("image/img1.png", "image/img2.png")
    alert(success)

async def test8():
    sha256 = await boss_hashfile("aaa/login.py")
    alert(sha256)
