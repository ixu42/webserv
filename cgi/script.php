<?php
echo "Content-type: text/html\n\n";
echo "<html><body>";
echo "<h1>PHP CGI Test</h1>";
echo "<p>This is a test of the PHP CGI functionality.</p>";
echo "<p>Query String: " . getenv('QUERY_STRING') . "</p>";
echo "<p>Request Method: " . getenv('REQUEST_METHOD') . "</p>";
echo "</body></html>";
?>
