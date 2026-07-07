#!/bin/bash
cd ui
npm run build
cd ..
cp ui/dist/assets/style.css.gz html/app.css.gz
cp ui/dist/assets/index.js.gz html/app.js.gz
        