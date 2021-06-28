<script>
    export let name;
    export let res;
    export let min = undefined;
    export let max = undefined;
    export let defaultVal = undefined;

    if (!res) res = defaultVal ? defaultVal : [0, 0];
</script>

<div class='holder'>
    <div class='name'>{name}</div>
    <div>
        <input type=number bind:value={res[0]} min={min} max={max}> <input type=number bind:value={res[1]} min={min} max={max}>
    </div>
</div>