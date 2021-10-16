$data = Get-ChildItem "$(Get-Location)\shaders" -Attributes !Directory | Where-Object { $_.Extension -eq ".vert" -or ".frag" } 

for ($i = 0; $i -lt $data.Count; $i++) {
    glslc.exe "$($data[$i].FullName)" -o "$($data[$i].Directory)\build\$($data[$i].Name).spv"
}