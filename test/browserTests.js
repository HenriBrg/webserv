const puppeteer = require('puppeteer');

let scrape = async () => {
    const browser = await puppeteer.launch({headless: true}); 
    const page = await browser.newPage();
    await page.goto('http://localhost:7777/');
    // await page.waitFor(100);
    var result = await page.content();
    browser.close(); // --> à retirer pour simuler un seul client avec plusieurs requêtes
    return result;
   };
   scrape().then((value) => {
    console.log(value); // complete HTML code of the target url!
   });