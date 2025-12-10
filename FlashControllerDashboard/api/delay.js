let currentDelay = 500;
const MIN_DELAY = 50;
const MAX_DELAY = 2000;

module.exports = (req, res) => {
  res.setHeader('Access-Control-Allow-Origin', '*');
  res.setHeader('Access-Control-Allow-Methods', 'GET,POST,OPTIONS');
  res.setHeader('Access-Control-Allow-Headers', 'Content-Type');

  if (req.method === 'OPTIONS') return res.status(200).end();

  if (req.method === 'GET') {
    return res.status(200).json({ delay: currentDelay });
  }

  if (req.method === 'POST') {
    try {
      const body = typeof req.body === 'string' ? JSON.parse(req.body) : req.body;
      let d = Number(body.delay);
      if (!Number.isFinite(d)) return res.status(400).json({ error: 'Invalid delay' });
      if (d < MIN_DELAY) d = MIN_DELAY;
      if (d > MAX_DELAY) d = MAX_DELAY;
      currentDelay = Math.round(d);
      return res.status(200).json({ delay: currentDelay });
    } catch (err) {
      return res.status(500).json({ error: 'Server error', details: String(err) });
    }
  }

  return res.status(405).json({ error: 'Method not allowed' });
};
