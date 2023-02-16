<!DOCTYPE html>
<html><body>
<?php
/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-esp8266-mysql-database-php/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

$servername = "localhost";

// REPLACE with your Database name
$dbname = "soil_data";
// REPLACE with Database user
$username = "soil_monitor";
// REPLACE with Database user password
$password = "Jonagu25!!";


// Create connection
$conn = new mysqli($servername, $username, $password, $dbname);
// Check connection
if ($conn->connect_error) {
    die("Connection failed: " . $conn->connect_error);
} 

$sql = "SELECT id, time, moisture FROM phineas_soil ORDER BY id DESC";

echo '<table cellspacing="5" cellpadding="5">
      <tr> 
        <td>Time</td> 
        <td>Moisture level</td>  
      </tr>';
 
if ($result = $conn->query($sql)) {
    while ($row = $result->fetch_assoc()) {
        $row_id = $row["ID"];
    //    $row_sensor = $row["sensor"];
    //   $row_location = $row["location"];
    //   $row_value1 = $row["value1"];
    //   $row_value2 = $row["value2"]; 
    //   $row_value3 = $row["value3"]; 
        $row_reading_time = $row["time"];
        // Uncomment to set timezone to - 1 hour (you can change 1 to any number)
        //$row_reading_time = date("Y-m-d H:i:s", strtotime("$row_reading_time - 1 hours"));
    $row_moisture = $row["moisture"];
        // Uncomment to set timezone to + 4 hours (you can change 4 to any number)
       // $row_reading_time = date("d-m-y H:i:s", strtotime("$row_reading_time + 4 hours"));
      
        echo '<tr> 
                <td>' . $row_reading_time . '</td> 
                <td>' . $row_moisture . '</td> 
              </tr>';
    }
    $result->free();
}

$conn->close();
?> 
</table>
</body>
</html>
