var cp = require('child_process');
cp.execSync('npm run dev', { stdio: [0, 1, 2] });

