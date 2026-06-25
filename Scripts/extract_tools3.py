import json

f = open(r'C:\Users\gdm\AppData\Local\Temp\trae\toolcall-output\899d78bb-ff0c-4f37-b66d-0fdeefbded12.txt', 'r', encoding='utf-8')
content = f.read()
f.close()

prefix = 'The MCP server responded with: '
idx = content.find(prefix) + len(prefix)
response = json.loads(content[idx:].strip())
data = json.loads(response[0]['text'])
tools = {t['name']: t for t in data['tools']}

# Write each tool to a separate file
for name in ['NiagaraToolsets.NiagaraToolset_System.CreateNiagaraSystem', 'NiagaraToolsets.NiagaraToolset_System.AddEmitter', 'NiagaraToolsets.NiagaraToolset_System.AddModule', 'NiagaraToolsets.NiagaraToolset_System.AddRenderer']:
    short_name = name.split('.')[-1]
    with open(f'E:\\uE5work\\Lr\\Scripts\\schema_{short_name}.json', 'w', encoding='utf-8') as out:
        json.dump(tools[name], out, indent=2, ensure_ascii=False)
    print(f'Wrote schema_{short_name}.json')