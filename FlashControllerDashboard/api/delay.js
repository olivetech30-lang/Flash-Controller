// ============================================
// Vercel Serverless API - /api/delay
// ============================================
// This endpoint handles GET and POST requests
// for the ESP32 Flash Controller System
// ============================================

// In-memory storage (Note: This resets on cold starts)
// For production, consider using Vercel KV, Redis, or a database
let currentDelay = 500;

// Configuration
const MIN_DELAY = 50;
const MAX_DELAY = 2000;
const DEFAULT_DELAY = 500;

// CORS headers for cross-origin requests
const corsHeaders = {
    'Access-Control-Allow-Origin': '*',
    'Access-Control-Allow-Methods': 'GET, POST, OPTIONS',
    'Access-Control-Allow-Headers': 'Content-Type',
    'Content-Type': 'application/json'
};

export default async function handler(req, res) {
    // Set CORS headers for all responses
    Object.entries(corsHeaders).forEach(([key, value]) => {
        res.setHeader(key, value);
    });

    // Handle preflight OPTIONS request
    if (req.method === 'OPTIONS') {
        return res.status(200).end();
    }

    // ============================================
    // GET Request - Fetch current delay
    // ============================================
    if (req.method === 'GET') {
        return res.status(200).json({
            delay: currentDelay,
            min: MIN_DELAY,
            max: MAX_DELAY,
            timestamp: Date.now()
        });
    }

    // ============================================
    // POST Request - Update delay value
    // ============================================
    if (req.method === 'POST') {
        try {
            const body = req.body;

            // Validate request body
            if (!body || typeof body.delay === 'undefined') {
                return res.status(400).json({
                    error: 'Missing delay value',
                    message: 'Request body must contain a "delay" field'
                });
            }

            // Parse and validate delay value
            let newDelay = parseInt(body.delay, 10);

            // Check if it's a valid number
            if (isNaN(newDelay)) {
                return res.status(400).json({
                    error: 'Invalid delay value',
                    message: 'Delay must be a valid integer'
                });
            }

            // Clamp the value to valid range
            newDelay = Math.max(MIN_DELAY, Math.min(MAX_DELAY, newDelay));

            // Update the stored delay
            currentDelay = newDelay;

            return res.status(200).json({
                delay: currentDelay,
                min: MIN_DELAY,
                max: MAX_DELAY,
                timestamp: Date.now(),
                message: 'Delay updated successfully'
            });

        } catch (error) {
            console.error('Error processing request:', error);
            return res.status(500).json({
                error: 'Internal server error',
                message: error.message
            });
        }
    }

    // ============================================
    // Method not allowed
    // ============================================
    return res.status(405).json({
        error: 'Method not allowed',
        message: 'Only GET and POST methods are supported'
    });
}
