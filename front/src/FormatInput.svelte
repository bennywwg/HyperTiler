<script>
    export let name;
    export let res;
    export let defaultVal = undefined;

    if (!res) res = defaultVal ? defaultVal : "";

    const checkFunc = formatString => {
        return formatString.search(/\{x[0-9]*\}/) != -1
        && formatString.search(/\{y[0-9]*\}/) != -1
        && formatString.search(/\{z[0-9]*\}/) != -1;
    };
    const makeFunc = formatString => {
        
    };
</script>

<div class='holder'>
    <div class='name'>{name + (checkFunc(res) ? "" : " (Malformed Format String)")}</div>
</div>
<div class='holder'>
    <input type=text bind:value={res} style="background-color: {!checkFunc(res) ? '#a44' : '#fff'}">
</div>

<style>
    input {
	    width: 100%;
    }
</style>