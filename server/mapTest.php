<!DOCTYPE html>
<html lang="en">

<head>
  <meta http-equiv="refresh" content="1">
  <meta charset="UTF-8">
  <title>Document</title>
  <link rel="stylesheet" href="../css/test.css">
</head>

<body>
  <?php

  define('DB_HOST', 'localhost');
  define('DB_USERNAME', 'root');
  define('DB_PASSWORD', '');
  define('DB_NAME', 'unitmonitor');

  enum cellType: int
  {
    case VOID = 0;
    case OBSTACLE = 1;
    case MIST = 2;
    case ERROR = 3;
    case DIFFERENT_POSSITION = 8;
    case SELF_POSITION = 9;
  }

  function printUnitMap(string $unitMap)
  {
    $mapCols = 6;
    $mapRows = 6;
    $counter = 0;

    echo ('<table>');

    for ($tr = 0; $tr < $mapRows; $tr++) {
      echo '<tr>';
      for ($td = 0; $td < $mapCols; $td++) {
        $cell = $unitMap[$counter];

        switch ($cell) {
          case (string)(cellType::VOID->value):
            $class = 'void';
            break;
          case (string)(cellType::OBSTACLE->value):
            $class = 'obstacle';
            break;
          case (string)(cellType::MIST->value):
            $class = "mist";
            break;

          case (string)(cellType::DIFFERENT_POSSITION->value):
            $class = 'different_car';
            break;

          case (string)(cellType::SELF_POSITION->value):
            $class = 'car';
            break;

          default:
            $class = 'mist';
            break;
        }

        echo ('<td class="' . $class . '"></td>');
        $counter++;
      }
      echo '</tr>';
    }

    echo ('</table>');
  }

  function printGeneralMap(string $map1, string $map2, string $map3)
  {
    (string) $generalMap = $map1;
    $mapCols = 6;
    $mapRows = 6;
    $counter = 0;

    for ($tr = 0; $tr < $mapRows; $tr++) {
      for ($td = 0; $td < $mapCols; $td++) {
        if (
          $map1[$counter] != (string)(cellType::VOID->value) &&
          $map1[$counter] != (string)(cellType::MIST->value) &&
          $map1[$counter] != (string)(cellType::OBSTACLE->value)
        ) {
          $map1[$counter] = (string)(cellType::VOID->value);
        }

        if (
          $map2[$counter] != (string)(cellType::VOID->value) &&
          $map2[$counter] != (string)(cellType::MIST->value) &&
          $map2[$counter] != (string)(cellType::OBSTACLE->value)
        ) {
          $map2[$counter] = (string)(cellType::VOID->value);
        }

        if (
          $map3[$counter] != (string)(cellType::VOID->value) &&
          $map3[$counter] != (string)(cellType::MIST->value) &&
          $map3[$counter] != (string)(cellType::OBSTACLE->value)
        ) {
          $map3[$counter] = (string)(cellType::VOID->value);
        }

        if (
          $map1[$counter] == (string)(cellType::VOID->value) ||
          $map2[$counter] == (string)(cellType::VOID->value) ||
          $map3[$counter] == (string)(cellType::VOID->value)
        ) {
          $generalMap[$counter] = (string)(cellType::VOID->value);
        } else if (
          $map1[$counter] == (string)(cellType::OBSTACLE->value) ||
          $map2[$counter] == (string)(cellType::OBSTACLE->value) ||
          $map3[$counter] == (string)(cellType::OBSTACLE->value)
        ) {
          $generalMap[$counter] = (string)(cellType::OBSTACLE->value);
        } else if (
          $map1[$counter] == (string)(cellType::MIST->value) &&
          $map2[$counter] == (string)(cellType::MIST->value) &&
          $map3[$counter] == (string)(cellType::MIST->value)
        ) {
          $generalMap[$counter] = (string)(cellType::MIST->value);
        }

        $counter++;
      }
    }
    echo ('General Map: ' . $generalMap . '<br/>');

    printUnitMap($generalMap);
  }

  $db = new mysqli(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_NAME);

  if ($db->connect_error) {
    die("Connection failed: " . $db->connect_error);
  }


  if (!empty($_GET['ID'])  && !empty($_GET['MAP'])) {
    $currentUnitID = $_GET['ID'];
    $currentSteps = $_GET['STEPS'];
    $currentMap = $_GET['MAP'];

    $sqlQ = "SELECT * FROM units WHERE ID = '$currentUnitID'";
    $stmt = $db->prepare($sqlQ);
    $stmt->execute();
    $result = $stmt->get_result()->fetch_assoc()['ID'];

    if (empty($result)) {
      $sqlQ = "INSERT INTO units (ID, STEPS, MAP) VALUES (?,?,?)";
      $stmt = $db->prepare($sqlQ);
      $stmt->bind_param("iis", $_GET['ID'], $_GET['STEPS'], $_GET['MAP']);
      $insert = $stmt->execute();
    } else {

      $sqlQ = "UPDATE units SET `STEPS` = '$currentSteps', `MAP` = '$currentMap' WHERE ID = '$currentUnitID';";
      $stmt = $db->prepare($sqlQ);
      $stmt->execute();
    }
    // $eventData = $result->fetch_assoc();
    // $event_id = $eventData['google_calendar_event_id'];
    // $db = new mysqli(DB_HOST, DB_USERNAME, DB_PASSWORD, DB_NAME);

    // if ($db->connect_error) {
    //   die("Connection failed: " . $db->connect_error);
    // }

    // $sqlQ = "INSERT INTO units (ID, STEPS, MAP) VALUES (?,?,?)";
    // $stmt = $db->prepare($sqlQ);
    // $stmt->bind_param("iis", $_GET['ID'], $_GET['STEPS'], $_GET['MAP']);
    // $insert = $stmt->execute();

    // switch ($_GET['ID']) {
    //   case 20:
    //     $_SESSION['ID_20'] = $_GET['ID'];
    //     $_SESSION['ID_20_STEPS'] = $_GET['STEPS'];
    //     $_SESSION['ID_20_MAP'] = $_GET['MAP'];
    //     break;

    //   case 21:
    //     $_SESSION['ID_21'] = $_GET['ID'];
    //     $_SESSION['ID_21_STEPS'] = $_GET['STEPS'];
    //     $_SESSION['ID_21_MAP'] = $_GET['MAP'];
    //     break;

    //   case 22:
    //     $_SESSION['ID_22'] = $_GET['ID'];
    //     $_SESSION['ID_22_STEPS'] = $_GET['STEPS'];
    //     $_SESSION['ID_22_MAP'] = $_GET['MAP'];
    //     break;

    //   default:
    //     echo ("IDI HAHUI");
    //     break;
    // }
    // echo ("MAP = {$_GET['MAP']}<br/>");
  } else {
  }

  for ($i = 20; $i < 23; $i++) {
    $sqlQ = "SELECT * FROM units WHERE ID = '$i'";
    $stmt = $db->prepare($sqlQ);
    $stmt->execute();
    $result = $stmt->get_result()->fetch_assoc();

    if (!empty($result)) {
      echo ("ID = ");
      echo ($result['ID']);
      echo ("<br/>");
      echo ("STEPS = ");
      echo ($result['STEPS']);
      echo ("<br/>");
      echo ("MAP = ");
      echo ($result['MAP']);
      printUnitMap($result['MAP']);
      echo ("<br/>");

      if ($i == 20) {
        $map20 = $result['MAP'];
      } else if ($i == 21) {
        $map21 = $result['MAP'];
      } else {
        $map22 = $result['MAP'];
        echo ("<br/>");
        printGeneralMap($map20, $map21, $map22);
      }
    }
  }

  /*
  if (!isset($_SESSION['map'])) {
    echo ('o hello there');
    $map = [];

    for ($i = 0; $i < 10; $i++) {
      $map[$i] = [];
      for ($j = 0; $j < 10; $j++) {
        $map[$i][$j] = 2;
      }
    }
  } else {
    $map = $_SESSION['map'];
  }

  $coordinats = 'Super:3:0:3:1:1:4:0:0:2:0:0';

  if (strlen($coordinats) == 33) {
    list($error, $location_x, $location_y, $x_1, $y_1, $flag_1, $x_2, $y_2, $flag_2, $x_3, $y_3, $flag_3, $x_4, $y_4, $flag_4) = explode(':', $coordinats);
    $map[$x_1][$y_1] = $flag_1;
    $map[$x_2][$y_2] = $flag_2;
    $map[$x_3][$y_3] = $flag_3;
    $map[$x_4][$y_4] = $flag_4;
  } else if (strlen($coordinats) == 27) {
    list($error, $location_x, $location_y, $x_1, $y_1, $flag_1, $x_2, $y_2, $flag_2, $x_3, $y_3, $flag_3) = explode(':', $coordinats);
    $map[$x_1][$y_1] = $flag_1;
    $map[$x_2][$y_2] = $flag_2;
    $map[$x_3][$y_3] = $flag_3;
  } else {
    list($error, $location_x, $location_y, $x_1, $y_1, $flag_1, $x_2, $y_2, $flag_2) = explode(':', $coordinats);
    $map[$x_1][$y_1] = $flag_1;
    $map[$x_2][$y_2] = $flag_2;
  }

  $_SESSION['map'] = $map;

  
*/
  ?>
</body>

</html>