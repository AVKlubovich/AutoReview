<?php
/*echo '<form action="" method="post" enctype="multipart/form-data"> 
 <p>type_command: <input type="text" name="type_command"></p>
 <p>json: <input type="text" name="json"></p>
 <p>Pos0: <input type="file" name="image"></p>
 <p>Pos1: <input type="file" name="pos1"></p>
 <p>Pos2: <input type="file" name="pos2"></p>
 <p>Pos3: <input type="file" name="pos3"></p>
 <p>Pos4: <input type="file" name="pos4"></p>
 <p>Pos5: <input type="file" name="pos5"></p>
 <p><input type="submit" value="sendData"></p> ';*/

require_once 'errors.php';
require_once 'config.php';
require_once 'utils.php';
require_once 'networkManager.php';
require_once 'dbManager.php';
require_once 'commands.php';

require_once './commands/base/baseCommand.php';
require_once './commands/base/basePhotos.php';
require_once './commands/base/baseFile.php';

require_once './commands/addDamage.php';


class Query
{
    public function run()
    {        
        if (empty($_POST) ||
            !isset($_POST["type_command"]) ||
            !isset($_POST["json"]))
        {
            Utils::printData(
                array('status' => Errors::NOT_SEND_FIELD,
                    'error' => Errors::instance()->data(Errors::NOT_SEND_FIELD),
                    't' => 1));
        }
        $type_command = $_POST["type_command"];
        
        $command = $this->getCommand($type_command);
        if (is_null($command))
        {
            Utils::printData(
                array('status' => Errors::NOT_SEND_FIELD,
                    'error' => Errors::instance()->data(Errors::NOT_SEND_FIELD),
                    't' => 2));
        }
        $assoc = json_decode(urldecode($_POST["json"]), true); //$_POST["json"]
        $command->exec($assoc);
        
        if (!array_key_exists('type_command', $assoc))
            $assoc["type_command"] = $type_command;

        $result = NetworkManager::sendJson(Config::HOST_CPP, $assoc);
        if ($result == Errors::NOT_SEND_DATA_TO_REMORE_SERVER)
        {
            Utils::printData(
                array('status' => $result, 
                'error' => Errors::instance()->data($result)));
        }
        
        Utils::printOk();
    }
    
    function getCommand($type_command)
    {
        if (Commands::ADD_FILE == $type_command)
            return new AddDamage();
        else
            return NULL;
    }
}

$query = new Query();
$query->run();

?>
