param(
    [string]$OutPath = "build\hellpit.ico"
)

$width = 32
$height = 32
$pixels = New-Object 'byte[]' ($width * $height * 4)

function ClampByte([int]$value) {
    if ($value -lt 0) { return 0 }
    if ($value -gt 255) { return 255 }
    return $value
}

function Set-Pixel([int]$x, [int]$y, [int]$r, [int]$g, [int]$b, [int]$a) {
    if ($x -lt 0 -or $x -ge $script:width -or $y -lt 0 -or $y -ge $script:height) { return }
    $index = (($script:height - 1 - $y) * $script:width + $x) * 4
    $script:pixels[$index + 0] = [byte](ClampByte $b)
    $script:pixels[$index + 1] = [byte](ClampByte $g)
    $script:pixels[$index + 2] = [byte](ClampByte $r)
    $script:pixels[$index + 3] = [byte](ClampByte $a)
}

function Blend-Pixel([int]$x, [int]$y, [int]$r, [int]$g, [int]$b, [double]$alpha) {
    if ($x -lt 0 -or $x -ge $script:width -or $y -lt 0 -or $y -ge $script:height) { return }
    $index = (($script:height - 1 - $y) * $script:width + $x) * 4
    $oldB = [int]$script:pixels[$index + 0]
    $oldG = [int]$script:pixels[$index + 1]
    $oldR = [int]$script:pixels[$index + 2]
    $oldA = [int]$script:pixels[$index + 3]
    $a = [Math]::Max(0.0, [Math]::Min(1.0, $alpha))
    $script:pixels[$index + 0] = [byte](ClampByte ([int]($oldB * (1.0 - $a) + $b * $a)))
    $script:pixels[$index + 1] = [byte](ClampByte ([int]($oldG * (1.0 - $a) + $g * $a)))
    $script:pixels[$index + 2] = [byte](ClampByte ([int]($oldR * (1.0 - $a) + $r * $a)))
    $script:pixels[$index + 3] = [byte](ClampByte ([int]([Math]::Max($oldA, 255 * $a))))
}

function Draw-PixelLine([int]$x0, [int]$y0, [int]$x1, [int]$y1, [int]$r, [int]$g, [int]$b, [int]$a) {
    $dx = [Math]::Abs($x1 - $x0)
    $dy = [Math]::Abs($y1 - $y0)
    $steps = [Math]::Max($dx, $dy)
    if ($steps -le 0) {
        Set-Pixel $x0 $y0 $r $g $b $a
        return
    }
    for ($i = 0; $i -le $steps; ++$i) {
        $t = $i / $steps
        $x = [int][Math]::Round($x0 + ($x1 - $x0) * $t)
        $y = [int][Math]::Round($y0 + ($y1 - $y0) * $t)
        Set-Pixel $x $y $r $g $b $a
    }
}

for ($y = 0; $y -lt $height; ++$y) {
    for ($x = 0; $x -lt $width; ++$x) {
        $dx = ($x - 15.5) / 15.2
        $dy = ($y - 15.5) / 15.2
        $d = [Math]::Sqrt($dx * $dx + $dy * $dy)
        if ($d -lt 0.98) {
            $rim = if ($d -gt 0.84) { 1 } else { 0 }
            if ($rim -eq 1) {
                Set-Pixel $x $y 7 4 9 255
            } else {
                $shade = [int](12 + (1.0 - $d) * 20)
                Set-Pixel $x $y (ClampByte ($shade + 22)) (ClampByte ($shade / 4)) (ClampByte ($shade / 2 + 8)) 255
            }
        }
    }
}

# HELL PIT icon: the in-game hook dagger cutting through the pit.
# This reads closer to the actual core mechanic than a generic demon eye.
for ($i = 0; $i -lt 5; ++$i) {
    $offset = $i - 2
    Draw-PixelLine (5 + $offset) 23 (25 + $offset) 8 58 0 10 210
}
Draw-PixelLine 5 22 25 7 245 26 38 255
Draw-PixelLine 6 24 26 9 128 0 18 255
Draw-PixelLine 8 23 24 11 255 92 58 255

# Chain / hook line from the player toward the blade.
Draw-PixelLine 4 26 17 16 64 54 72 255
Draw-PixelLine 5 26 18 16 154 130 132 255
for ($i = 0; $i -lt 5; ++$i) {
    $cx = 5 + $i * 3
    $cy = 25 - $i * 2
    Set-Pixel $cx $cy 220 194 170 255
    Set-Pixel ($cx + 1) $cy 48 38 48 255
}

# Dagger body, matching the small hook sprite and slash effect in-game.
Draw-PixelLine 12 21 24 9 12 10 16 255
Draw-PixelLine 13 22 25 10 12 10 16 255
Draw-PixelLine 14 21 26 9 218 194 166 255
Draw-PixelLine 15 22 27 10 120 104 112 255
Draw-PixelLine 16 20 24 12 255 236 204 255

# Hook barb / claw tip.
Draw-PixelLine 23 9 28 6 228 210 184 255
Draw-PixelLine 24 10 29 10 228 210 184 255
Draw-PixelLine 28 6 26 13 24 18 24 255
Draw-PixelLine 29 10 24 14 24 18 24 255
Draw-PixelLine 24 14 20 13 228 210 184 255

# Purple dash-orb ember behind the hook, tying the icon to traversal.
for ($y = 10; $y -le 18; ++$y) {
    for ($x = 6; $x -le 14; ++$x) {
        $dx = ($x - 10.0) / 4.0
        $dy = ($y - 14.0) / 4.0
        if ($dx * $dx + $dy * $dy -lt 1.0) {
            Blend-Pixel $x $y 158 64 236 0.54
        }
    }
}
Set-Pixel 10 14 236 178 255 255
Set-Pixel 11 13 206 112 255 255

function Scale-IconPixels([int]$targetSize) {
    $scaled = New-Object 'byte[]' ($targetSize * $targetSize * 4)
    for ($y = 0; $y -lt $targetSize; ++$y) {
        for ($x = 0; $x -lt $targetSize; ++$x) {
            $sx = [int][Math]::Floor($x * $script:width / $targetSize)
            $sy = [int][Math]::Floor($y * $script:height / $targetSize)
            if ($sx -ge $script:width) { $sx = $script:width - 1 }
            if ($sy -ge $script:height) { $sy = $script:height - 1 }
            $source = (($script:height - 1 - $sy) * $script:width + $sx) * 4
            $dest = (($targetSize - 1 - $y) * $targetSize + $x) * 4
            $scaled[$dest + 0] = $script:pixels[$source + 0]
            $scaled[$dest + 1] = $script:pixels[$source + 1]
            $scaled[$dest + 2] = $script:pixels[$source + 2]
            $scaled[$dest + 3] = $script:pixels[$source + 3]
        }
    }
    return ,$scaled
}

$andStride = [int]([Math]::Floor(($width + 31) / 32) * 4)
$andMask = New-Object 'byte[]' ($andStride * $height)
$imageSize = 40 + $pixels.Length + $andMask.Length

$directory = Split-Path -Parent $OutPath
if ($directory -and !(Test-Path $directory)) {
    New-Item -ItemType Directory -Path $directory | Out-Null
}

$stream = [System.IO.File]::Open($OutPath, [System.IO.FileMode]::Create, [System.IO.FileAccess]::Write)
$writer = New-Object System.IO.BinaryWriter($stream)
try {
    $writer.Write([UInt16]0)
    $writer.Write([UInt16]1)
    $writer.Write([UInt16]1)
    $writer.Write([byte]$width)
    $writer.Write([byte]$height)
    $writer.Write([byte]0)
    $writer.Write([byte]0)
    $writer.Write([UInt16]1)
    $writer.Write([UInt16]32)
    $writer.Write([UInt32]$imageSize)
    $writer.Write([UInt32]22)

    $writer.Write([UInt32]40)
    $writer.Write([Int32]$width)
    $writer.Write([Int32]($height * 2))
    $writer.Write([UInt16]1)
    $writer.Write([UInt16]32)
    $writer.Write([UInt32]0)
    $writer.Write([UInt32]($pixels.Length))
    $writer.Write([Int32]0)
    $writer.Write([Int32]0)
    $writer.Write([UInt32]0)
    $writer.Write([UInt32]0)
    $writer.Write($pixels)
    $writer.Write($andMask)
}
finally {
    $writer.Close()
    $stream.Close()
}
