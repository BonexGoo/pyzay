from js import alert, boss_py2js

args = ["###BOSS_ARGS###"]
alert("[args] " + str(args))

def boss_js2py(pid, text):
    alert("[js2py] " + text)
    boss_py2js(pid, "set,d.key1,123")
