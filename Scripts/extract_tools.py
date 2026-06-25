import json

f = open(r'C:\Users\gdm\AppData\Local\Temp\trae\toolcall-output\899d78bb-ff0c-4f37-b66d-0fdeefbded12.txt', 'r', encoding='utf-8')
content = f.read()
f.close()

prefix = 'The MCP server responded with: '
idx = content.find(prefix) + len(prefix)
response = json.loads(content[idx:].strip())
data = json.loads(response[0]['text'])
tools = {t['name']: t for t in data['tools']}

names = [
    'NiagaraToolsets.NiagaraToolset_System.AddEmitter',
    'NiagaraToolsets.NiagaraToolset_System.AddModule',
    'NiagaraToolsets.NiagaraToolset_System.AddRenderer',
    'NiagaraToolsets.NiagaraToolset_System.SetEmitterData',
    'NiagaraToolsets.NiagaraToolset_System.SetStackInputData',
    'NiagaraToolsets.NiagaraToolset_System.SetRendererData',
]

for name in names:
    print('=== ' + name + ' ===')
    print(json.dumps(tools[name], indent=2, ensure_ascii=False)[:3000])
    print()