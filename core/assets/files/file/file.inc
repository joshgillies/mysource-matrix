<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';
require_once SQ_FUDGE_PATH.'/general/file_system.inc';

/**
* File
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package Resolve::packages::__core__
*/
class File extends Asset
{

	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function File($assetid=0)
	{
		$this->Asset($assetid);
	}//end constructor


	/**
	* Create this asset
	*
	* @param string	$name	the name of this file
	*
	* @return boolean
	* @access public
	*/
	function create($name)
	{
		$name = trim($name);

		if ($name == '') {
			trigger_error('Unable to create a File with a blank name', E_USER_WARNING);
			return false;
		}

		if (parent::create()) {

			// short and long names are the same to begin with
			$this->setAttrValue('short_name', $name);
			$this->setAttrValue('name', $name);
			
			return true;

		}//end if

		return false;

	}//end create()


	/**
	* Returns an array of all the permitted link type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		return Array(SQ_LINK_EXCLUSIVE => Array(),
					 SQ_LINK_UNITE     => Array('thumbnail' => 'M'),
					 SQ_LINK_USES      => Array(),
					 SQ_LINK_NOTICE    => Array()
					);

	}//end _getAllowedLinks()


	/**
	* Processes a backend submission from this asset, returns true if all OK
	*
	* @return boolean
	* @access public
	*/
	function processBackend()
	{
		switch($_POST['asset_action']) {
			case 'create' :
				$ei = &$this->getEI();
				$ei->process($this, true);

				$name = $this->getAttribute('name');
				if ($name->processed) {
					return $this->create($name->value);
				} else {
					return false;
				}
				break;

			default :
				return parent::processBackend();

		}// end switch

	}// end processBackend()


	/**
	* Prints out the frontend for this file with no design
	*
	* @return void
	* @access public
	*/
	function printFrontend()
	{
		$existing = $this->_getExistingFile();
		if (!empty($existing)) {
			$stats = getimagesize($existing['path']);
			header('Cache-Control:');
			header('Pragma: cache');
			header('Expires: '. gmdate('D, d M Y H:i:s',time()  +86400) . ' GMT');
			header('Content-Type: ' . $stats['mime']);
			header('Content-Disposition: inline; filename='.$existing['filename'].';');
			header('Last-Modified: ' . gmdate('D, d M Y H:i:s',$existing['modified']) . ' GMT');
			header('Content-Length: '.$existing['size']);
			readfile($existing['path']);
		}
	}// end printFrontend()


	/**
	* Get an array of some useful data about the existing uploaded file
	*
	* @return array(string)
	* @access public
	*/
	function _getExistingFile()
	{
		// check for an existing file
		$existing = Array();
		$existing_files = list_files($this->data_path);
		foreach ($existing_files as $filename) {
			if (preg_match('/^uploaded_file(.[^.]+)?$/', $filename)) {
				$existing['path'] = $this->data_path.'/'.$filename;
				$existing['filename'] = $filename;
				$existing['modified'] = filectime($existing['path']);
				$existing['size'] = filesize($existing['path']);
				break;
			}
		}
		return $existing;
	}//end _getExistingFile()


	/**
	* Returns name of the asset
	*
	* @param boolean	$short_name	whether or not we are after the short name or the full name
	*
	* @see Asset::_getName()
	* @access public
	*/
	function _getName($short_name=false)
	{
		return $this->attr('name');

	}//end _getName();


}//end class
?>
