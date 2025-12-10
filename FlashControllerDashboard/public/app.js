const API_BASE = '/api/delay';
const slider = document.getElementById('slider');
const incBtn = document.getElementById('incBtn');
const decBtn = document.getElementById('decBtn');
const delayVal = document.getElementById('delayVal');
const STEP = 50;

async function fetchDelay() {
  try {
    const res = await fetch(API_BASE);
    const data = await res.json();
    slider.value = data.delay;
    delayVal.textContent = data.delay;
  } catch (err) { console.error(err); }
}

async function sendDelay(d) {
  try {
    await fetch(API_BASE, { method:'POST', headers:{'Content-Type':'application/json'}, body:JSON.stringify({delay:d}) });
    delayVal.textContent = d;
  } catch(err){console.error(err);}
}

// Slider input
slider.addEventListener('input', e => delayVal.textContent = e.target.value);
slider.addEventListener('change', e => sendDelay(Number(e.target.value)));

// Buttons
incBtn.addEventListener('click', () => {
  let v = Number(slider.value)+STEP; if(v>2000)v=2000; slider.value=v; sendDelay(v);
});
decBtn.addEventListener('click', () => {
  let v = Number(slider.value)-STEP; if(v<50)v=50; slider.value=v; sendDelay(v);
});

// Poll backend every second
setInterval(fetchDelay, 1000);
fetchDelay();
