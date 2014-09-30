<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.net) so we may provide   |
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: regenerate_file_system.php,v 1.2 2012/10/19 05:05:15 cupreti Exp $
*
*/

/**
 * Regenerate asset file system data by rootnode/type
 * Especially useful for broken file systems after a character encoding change.
 *
 * @author  Matthew Spurrier <mspurrier@squiz.net>
 * @version $Revision: 1.2 $
 * @package MySource_Matrix
**/
error_reporting(E_ALL ^ E_DEPRECATED ^ E_STRICT);
/**
 * Error Handler
 *
 * @author Matthew Spurrier
**/
class MErrorHandler
{
	public $errorLogFile;

	/**
	 * Error handler constructor
	 * Set this class as the error handler and set log file
	 *
	 * @access public
	**/
	public function __construct()
	{
		set_error_handler(array($this,'throwError'));
		set_exception_handler(array($this,'throwException'));
		$this->errorLogFile=__FILE__ . '.log';
	}

	/**
	 * Error handler destructor
	 * Restore error handling
	 *
	 * @access public
	**/
	public function __destruct()
	{
		restore_error_handler();
		restore_exception_handler();
	}

	/**
	 * Throw error
	 *
	 * @param $errorNumber (int) Error Number
	 * @param $errorMessage (string) Error Message
	 * @param $errorFile (string) File the error occurred in
	 * @param $errorLine (int) Line the error occurred on
	 * @access public
	**/
	public function throwError($errorNumber,$errorMessage,$errorFile,$errorLine) {
	
		$current_level = error_reporting();

		// this error level is not set, so dont show the error, like all those php strict errors
        if (($errorNumber & $current_level) == 0) return;

		switch ($errorNumber) {
			case E_USER_ERROR:
				$type="ERROR";
				break;
			case E_USER_WARNING:
				$type="WARNING";
				break;
			case E_USER_NOTICE:
				$type="NOTICE";
				break;
			default:
				$type="UNKNOWN";
				break;
		};
		$this->output(sprintf('%s - \'%s\' on line %d of %s',$type,$errorMessage,$errorLine,$errorFile));
		if ($errorNumber == E_USER_ERROR) exit(1);
	}

	/**
	 * Throw exception
	 *
	 * @param $exception (object) Exception Object
	 * @access public
	**/
	public function throwException($exception) {
		$this->output(sprintf('Uncaught exception \'%s\' on line %d of %s',$exception->getMessage(),$exception->getLine(),$exception->getFile()));
		exit(255);
	}

	/**
	 * Output error message
	 * Write error to log file, if that fails, print the error to the screen
	 *
	 * @param $message (string) Error Message
	 * @access public
	**/
	public function output($message) {
		$message=sprintf('[%s] %s'."\n",date('d/m/Y H:i:s'),$message);
		if (!file_put_contents($this->errorLogFile,$message,FILE_APPEND)) {
            while (ob_get_level()) {
                ob_end_flush();
            }
			print $message;
		}
	}
}

/**
 * Process Controller
 *
 * @author Matthew Spurrier
**/
class MProcessController
{
	protected $shutdown;
	protected $children;
	protected $signalQueue;
	protected $parentPID;


	/**
	 * Process Controller Setup
	 *
	 * @access protected
	**/
	protected function setup()
	{
		declare(ticks = 1);
		pcntl_signal(SIGCHLD,array($this,'signalHandler'));
		register_tick_function(array($this,'checkExit'));
		$this->shutdown=false;
		$this->children=array();
		$this->parentPID=getmypid();
	}


	/**
	 * Instantiate Process controller and setup
	 *
	 * @return object
	 * @disposition static
	 * @access public
	**/
	public static function get()
	{
		$pc=new self;
		$pc->setup();
		return $pc;
	}


	/**
	 * Set shutdown
	 *
	 * @access public
	**/
	public function shutdown()
	{
		$this->shutdown=true;
	}


	/**
	 * Signal Handler
	 *
	 * @param $signal (int) Signal ID
	 * @param $pid (int) Process ID, default: null
	 * @return boolean
	 * @access public
	**/
	public function signalHandler($signal,$pid=null)
	{
		if(!$pid){
		    $pid = pcntl_waitpid(-1, $status, WNOHANG);
		}
		while ($pid > 0) {
			if ($pid) {
				$key=array_search($pid,$this->children);
				unset($this->children[$key]);
			}
			$pid = pcntl_waitpid(-1, $status, WNOHANG);
		}
		return true;
	}


	/**
	 * Check Exit
	 * Allows for ticks to shut down children
	 *
	 * @access public
	**/
	public function checkExit()
	{
		if ($this->shutdown) exit;
	}


	/**
	 * Fork Process
	 *
	 * @return int or boolean
	 * @access public
	**/
	public function fork()
	{
		$child_pid = pcntl_fork();
		switch ($child_pid) {
			case -1:
				trigger_error("Forking failed!",E_USER_ERROR);
				return false;
				break;
			case 0:
				return true;
				break;
			default:
				$status = null;
				$this->children[]=$child_pid;
				return $child_pid;
				break;
		}
	}


	/**
	 * Retrieve count of running children
	 *
	 * @return int
	 * @access public
	**/
	public function childCount()
	{
		return count($this->children);
	}


	/**
	 * Check if child is marked as running
	 *
	 * @param $pid (int) Process ID
	 * @return boolean
	 * @access public
	**/
	public function childRunning($pid)
	{
		return in_array($pid,$this->children);
	}

}//end class


/**
 * Regenerate Matrix Filesystem
 *
 * @author Matthew Spurrier
**/
class MatrixRegenFS
{
	protected $pcntl;
	protected $systemRoot;
	protected $processDesigns;
	protected $processMetadata;
	protected $processBodycopies;
	protected $rootNode;


	/**
	 * Script setup
	 * Make sure we have all the required arguments, setup process controls, etc
	 *
	 * @access public
	**/
	public function setup()
	{
		if ((php_sapi_name() != 'cli')) {
			echo "ERROR: You can only run this script from the command line\n";
			exit(1);
		}
		$SYSTEM_ROOT = $this->getCLIArg('system');
		if (!$SYSTEM_ROOT) {
			echo "ERROR: You need to supply the path to the System Root\n";
			$this->printUsage();
		}
		if (!is_dir($SYSTEM_ROOT) || !is_readable($SYSTEM_ROOT.'/core/include/init.inc')) {
			echo "ERROR: Path provided doesn't point to a Matrix system root directory\nPlease provide correct path and try again.\n";
			$this->printUsage();
		}
		$this->systemRoot=$SYSTEM_ROOT;
		$ROOTNODE=$this->getCLIArg('rootnode');
		if ($ROOTNODE) {
			if (is_numeric($ROOTNODE)) {
				$this->rootNode=$ROOTNODE;
			} else {
				echo "ERROR: Root node provided is not numeric\n";
				$this->printUsage();
			}
		} else {
			$this->rootNode=1;
		}
		if ($this->getCLIArg('all')) {
			$this->processDesigns=true;
			$this->processMetadata=true;
			$this->processBodycopies=true;
		} else {
			$this->processDesigns=($this->getCLIArg('designs'))?true:false;
			$this->processMetadata=($this->getCLIArg('metadata'))?true:false;
			$this->processBodycopies=($this->getCLIArg('bodycopies'))?true:false;
		}
		if ((!$this->processDesigns) && (!$this->processMetadata) && (!$this->processBodycopies)) $this->printUsage();
		$this->pcntl=MProcessController::get();
	}


	/**
	 * Class destructor
	 *
	 * @access public
	**/
	public function __destruct()
	{
		// If we're using process control, send a shutdown signal to our children
		if ($this->pcntl) {
			$this->pcntl->shutdown();
		}
	}


	/**
	 * Set up matrix environment
	 *
	 * @access public
	**/
	public function matrixSetup()
	{
		ini_set('memory_limit','256M');
		require_once $this->systemRoot.'/core/include/init.inc';
		$eh=new MErrorHandler;
		$root_user = $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('root_user');
		$GLOBALS['SQ_SYSTEM']->setCurrentUser($root_user);
	}


	/**
	 * Print Usage and Exit
	 *
	 * @access protected
	**/
	protected function printUsage()
	{
		$FILE=basename(__FILE__);
		$usage=<<<USAGE
Matrix Filesystem Regeneration
php %s --system=<SYSTEM_ROOT> [--rootnode=<ROOT_NODE>] ([--designs] [--metadata] [--bodycopies] | --all)
	--system=<SYSTEM_ROOT>	: Matrix file system root directory
	--rootnode=<ROOT_NODE>	: Root node you wish to generate from
	--designs					: Regenerate designs only
	--metadata				: Regenerate metadata only
	--bodycopies				: Regenerate bodycopies only
	--all						: Regenerate designs, metadata, and bodycopies

EG: If you want to regenerate designs only on the whole system, use:
php %s --system=<SYSTEM_ROOT> --designs

For designs and metadata only on root node 25, use:
php %s --system=<SYSTEM_ROOT> --designs --metadata --rootnode=25

To run all regeneration steps over the whole system, use:
php %s --system=<SYSTEM_ROOT> --all


USAGE;
		printf($usage,$FILE,$FILE,$FILE,$FILE);
		exit(1);
	}


	/**
	 * Check if a CLI argument exists and if it has a value
	 * If it has a value, return it, otherwise return true as the argument is set.
	 * If the argument does not exist, return false
	 *
	 * @param $arg (string) expected ommand line argument string
	 * @return string or boolean
	 * @access protected
	**/
	protected function getCLIArg($arg)
	{
		return (count($match = array_values(preg_grep("/--" . $arg . "(\=(.*)|)/i",$_SERVER['argv']))) > 0 === TRUE) ? ((preg_match('/--(.*)=(.*)/',$match[0],$reg)) ? $reg[2] : true) : false;
	}


	/**
	 * Retrieve data from matrix in order to perform tasks
	 * Data retrieved is relevant to the current run settings and gives the
	 * worker functions the information required to process the request
	 *
	 * @return array
	 * @access protected
	**/
	protected function getAssetInfo()
	{
		$file='/tmp/regenfs-assetdata-' . time() . '.tmp';
		$retrievingTemplate='Retrieving Asset Data'."\t\t\t".'%s   %s';
		printf($retrievingTemplate,'....',"\r");
		// Start a new child process
		$child=$this->pcntl->fork();
		if ($child === true) {
			// We're the child, let's do some work
			$this->matrixSetup();
			// Ensure Root node is a valid asset or die
			if (!$GLOBALS['SQ_SYSTEM']->am->assetExists($this->rootNode)) {
				trigger_error(sprintf('Root node \'%s\' does not exist!',$this->rootNode),E_USER_WARNING);
				posix_kill($this->pcntl->parentID,9);
				exit(1);
			}
			// Get all contexts
			$contextids = array_keys($GLOBALS['SQ_SYSTEM']->getAllContexts());
			$assets=array();
			// If we're processing designs, include them on the list
			if ($this->processDesigns) $assets['design']=array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($this->rootNode,array('design','design_css'),true));
			// If we're processing metadata, include them on the list
			if ($this->processMetadata) {
				$assets['metadata']=array();
				$nodeassets=array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($this->rootNode));
				$nodeassets=array_chunk($nodeassets,100);
				foreach ($nodeassets as $k => $entries) {
					$keys=array_keys($entries);
					// Add binding character
					array_walk($keys,create_function('&$v,$k','$v=\':a\'.$v;'));
					// Implode key array into sql statement for pdo processing
					$sql='select distinct assetid from sq_ast_mdata where assetid in ('.implode(',',$keys).')';
					unset($keys);
					$query = MatrixDAL::preparePdoQuery($sql);
					// Bind keys to entry
					foreach ($entries as $kk => $entry) {
						MatrixDAL::bindValueToPdo($query,'a'.$kk,$entry);
					}
					$metadata=MatrixDAL::executePdoAssoc($query);
					foreach($metadata as $k => $row) {
						$assets['metadata'][]=$row['assetid'];
					}
				}
			}
			// If we're processing bodycopies, include them on the asset list
			if ($this->processBodycopies) {
				$content_type_assetids=array_keys($GLOBALS['SQ_SYSTEM']->am->getChildren($this->rootNode,'content_type',false));
				$assets['bodycopy']=array();
				foreach($content_type_assetids as $assetid) {
					$bodycopy_container_link = $GLOBALS['SQ_SYSTEM']->am->getLinks($assetid, SQ_LINK_TYPE_2, Array('bodycopy_container'), FALSE, 'minor');
					if (isset($bodycopy_container_link[0]['majorid'])) $assets['bodycopy'][] = $bodycopy_container_link[0]['majorid'];
				}
			}
			$output=array();
			$output['contextids']=$contextids;
			$output['assets']=$assets;
			// Output data to the data file
			file_put_contents($file,serialize($output));
			exit(0);
		} else if (is_numeric($child)) {
			// We're the parent, let's rest while the child is doing the chores
			while ($this->pcntl->childRunning($child) === true) {
				sleep(1);
			}
			// Child has done it's chores, check to see if the file it needs to create is there, then process it
			if (file_exists($file)) {
				// File is there, get the file contents and then delete the file
				$response=file_get_contents($file);
				unlink($file);
				$response=@unserialize($response);
				printf($retrievingTemplate,'[DONE]',"\n");
				if (is_array($response)) {
					return $response;
				} else {
					return array();
				}
			}
		}
	}


	/**
	 * Perform Regeneration
	 * Primary worker function - collect asset list and iterate through cnotexts and asset types
	 * Send the asset id's for the relevant types along with the context id to the processing function
	 *
	 * @access public
	**/
	public function performRegeneration()
	{
		$assetData=$this->getAssetInfo();
		if (is_array($assetData['contextids']) && is_array($assetData['assets'])) {
			foreach ($assetData['contextids'] as $k => $contextid) {
				foreach ($assetData['assets'] as $type => $assets) {
					$this->processAssets($assets,$contextid,$type);
				}
			}
		}
	}


	/**
	 * Process Assets
	 *
	 * @param $assets (array) Asset List
	 * @param $contextid (int) Context ID
	 * @param $type (string) Asset type to determine what work to do on it
	 * @return boolean
	 * @access protected
	**/
	protected function processAssets($assets,$contextid,$type)
	{
		switch ($type) {
			case 'design':
					// Processing a design, we'd like to do this one at a time please
					$ttype="Designs";
					$clusterSize=1;
					break;
			case 'metadata':
					// Processing metadata, we can do this 5 at a time
					$ttype="Metadata";
					$clusterSize=5;
					break;
			case 'bodycopy':
					// Processing bodycopies, we can do this 5 at a time
					$ttype="Bodycopies";
					$clusterSize=5;
					break;
			default:
					// Unexpected result, trigger warning message
					trigger_error(sprintf('Type \'%s\' not recognised, this shouldn\'t happen, something is wrong!',$type),E_USER_WARNING);
					return false;
					break;
		}
		$procTpl=sprintf('Processing %s'."\t\t\t",$ttype);
		$procTpl_Status='%s%.2f%%   '."\r";
		$procTpl_Done='%s[DONE]   '."\n";
		printf($procTpl_Status,$procTpl,0);
		$i=0;
		$count=count($assets);
		// Split the work into chunks to preserve system resources
		$assetCluster=array_chunk($assets,$clusterSize);
		// For each of the chunks, start a child process and process the work
		foreach ($assetCluster as $cluster => $assetData) {
			// Odd, the chunk isn't an array, continue to the next one
			if (!is_array($assetData)) continue;
			// Start a new child process
			$child=$this->pcntl->fork();
			if ($child === true) {
				// We're the child, let's do some work
				/// Setup matrix environment
				$this->matrixSetup();
				$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
				$GLOBALS['SQ_SYSTEM']->setRunLevel(SQ_RUN_LEVEL_FORCED);
				$GLOBALS['SQ_SYSTEM']->changeContext($contextid);
				$mm=$GLOBALS['SQ_SYSTEM']->getMetadataManager();
				// For each of the assets run the required process over them depending on their type
				foreach ($assetData as $k => $assetid) {
					// If this asset does not actually exist, continue on to the next one
					if (!$GLOBALS['SQ_SYSTEM']->am->assetExists($assetid)) continue;
					$asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
					if (is_null($asset)) continue;
					switch ($type) {
						case 'bodycopy':
							// Regenerate the bodycopy file content
							$bodycopy_container_edit_fns = $asset->getEditFns();
							$bodycopy_container_edit_fns->generateContentFile($asset);
							break;
						case 'metadata':
							// Regenerate the metadata
							$mm->regenerateMetadata($assetid, NULL, FALSE);
							break;
						case 'design':
							// If we're not a design for some reason, continue
							if (!($asset instanceof Design)) continue;
							$design_edit_fns = $asset->getEditFns();
							// Parse and process the design, if successful generate the design file
							if (@$design_edit_fns->parseAndProcessFile($asset)) @$asset->generateDesignFile(false);
							// Update respective design customisations
							$customisation_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($assetid, SQ_LINK_TYPE_2, 'design_customisation', true, 'major', 'customisation');
							foreach($customisation_links as $link) {
								$customisation = $GLOBALS['SQ_SYSTEM']->am->getAsset($link['minorid'], $link['minor_type_code']);
								if (is_null($customisation)) continue;
								@$customisation->updateFromParent($design);
								$GLOBALS['SQ_SYSTEM']->am->forgetAsset($customisation);
							}
							break;
						default:
							break;
					}
					$asset = $GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				}
				$GLOBALS['SQ_SYSTEM']->restoreContext();
				$GLOBALS['SQ_SYSTEM']->restoreRunLevel();
				$GLOBALS['SQ_SYSTEM']->restoreCurrentUser();
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				exit(0);
			} else if (is_numeric($child)) {
				// We're the parent, let's rest while the child is doing the chores
				while ($this->pcntl->childRunning($child) === true) {
					sleep(1);
				}
			}
			$i+=count($assetData);
			// Output current process completion
			printf($procTpl_Status,$procTpl,(($i/$count)*100));
		}
		// Output process status as done
		printf($procTpl_Done,$procTpl);
		return true;
	}

}//end class


// Start error handler
$eh=new MErrorHandler;
// Start matrix regeneration
$a=new MatrixRegenFS;
// Setup regenerator
$a->setup();
// Perform the regeneration
$a->performRegeneration();

?>
