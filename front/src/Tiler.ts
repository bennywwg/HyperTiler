const UserDownloadText = (content, fileName) => {
    var a = document.createElement("a");
    var file = new Blob([content], {type: 'text/plain'});
    a.href = URL.createObjectURL(file);
    a.download = fileName;
    a.click();
    URL.revokeObjectURL(a.href);
}

const CheckTileExistence = (formatStr, coord) => {
    return new Promise((resolve, reject) => {
        const req = new XMLHttpRequest();
        req.open("POST", "/exists");
        req.setRequestHeader("Content-Type", "application/json;charset=UTF-8");
        req.onreadystatechange = (e) => {
            if (e.target.readyState != XMLHttpRequest.DONE) return;
            let val;
            try {
                val = JSON.parse(e.target.response);
            } catch {
                reject();
                return;
            }
            resolve(val);
        };
        req.send(JSON.stringify({
            "formatStr": formatStr,
            "coord":coord
        }));
    });
};

const GenerateManifest = (formatStr, beginCoord, endCoord, maxInFlight, remaining) => {
    return new Promise((resolve, reject) => {
        let list = [];
        let toCheck = [];

        for(let z = beginCoord[2]; z < endCoord[2]; ++z) {
            for(let y = beginCoord[1]; y < endCoord[1]; ++y) {
                for(let x = beginCoord[0]; x < endCoord[0]; ++x) {
                    toCheck.push([x, y, z]);
                }
            }
        }

        remaining.value = toCheck.length;

        const CheckThenContinue = () => {
            if (toCheck.length === 0) return;

            const coord = toCheck.pop();

            CheckTileExistence(formatStr, coord)
            .then(val => {
                if (val) list.push(coord);
            })
            .finally(() => {
                CheckThenContinue();
                remaining.value = toCheck.length;
                if (toCheck.length === 0) resolve(list);
            });
        };

        for (let i = 0; i < maxInFlight; ++i) CheckThenContinue();
    });
};