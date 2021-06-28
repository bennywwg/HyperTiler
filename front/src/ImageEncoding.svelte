<script>
    export let name;
</script>

{name}
<div class='holder'>
    <div class='name'>Intensity Encoding</div>
    <select>
        <option>Auto</option>
        <option>sRGB</option>
        <option>Linear</option>
    </select>
</div>

<div class='holder'>
    <div class='name'>Bit Depth</div>
    <select>
        <option>Auto</option>
        <option>16</option>
    </select>
</div>

<style>
    select {
        width: 10em;
        height: 2em;
    }
</style>