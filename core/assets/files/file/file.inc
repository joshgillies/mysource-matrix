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
* @package Resolve_Packages
* @subpackage __core__
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
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return int|false
	* @access public
	*/
	function create(&$link)
	{
		$name = trim($this->attr('name'));

		if ($name == '') {
			trigger_error('Unable to create a '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name').' with a blank name', E_USER_WARNING);
			return false;
		}

		// check that a file was uploaded at least and that it succeeded
		$ei = $this->getEI();
		$info = get_file_upload_info($ei->getPrefix($this));
		if ($info === false || empty($info)) {
			trigger_error('File upload failed. Unable to create '.$GLOBALS['SQ_SYSTEM']->am->getTypeInfo($this->type(), 'name'), E_USER_WARNING);
			return false;
		}

		return parent::create($link);

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
					 SQ_LINK_UNITE     => Array(),
					 SQ_LINK_USES      => Array('thumbnail' => '1'),
					 SQ_LINK_NOTICE    => Array()
					);

	}//end _getAllowedLinks()


	/**
	* Processes a backend submission from this asset, returns true if all OK
	*
	* @param object Backend_Outputter	$o
	* @param array(string)				&$link	information used to create the initial link
	*
	* @return boolean
	* @access public
	*/
	function processBackend(&$o, &$link)
	{
		switch($_POST['asset_action']) {
			case 'create' :
				$this->_tmp['uploading_file'] = 1;
				$ei = &$this->getEI();
				if (!$ei->process($this, $o, true)) return false;

				$prefix = $ei->getPrefix($this);

				$this->setAttrValue('name', 'temp');
				$linkid = $this->create($link);

				if ($linkid) {
					$edit = $this->getEditFns();
					unset($this->_tmp['uploading_file']);
					return $edit->processFileUpload($this, $o, $prefix);
				} else {
					return false;
				}
				break;

			default :
				return parent::processBackend($o, $link);

		}// end switch

	}// end processBackend()


	/**
	* Set the value for the passed variable to the passed value
	* If the user changes the name of this file, we also need
	* to update the file system
	*
	* @param string		$name				the name of the attribute
	* @param string		$value				the new value of the attribute
	* @param boolean	$dont_run_updated	if true this->_updated() won't be run (THIS IS ONLY EVER CALLED FROM Asset::create())
	*
	* @return boolean
	* @access public
	*/
	function setAttrValue($name, $value, $dont_run_updated=false)
	{
		$uploading_file = (!empty($this->_tmp['uploading_file'])) ? true : false;
		if ($name == 'name' && !$uploading_file) {
			$old_filename = $this->attr('name');
			$old_ext = strtolower(substr(strrchr($old_filename,'.'), 1));
			$new_ext = strtolower(substr(strrchr($value,'.'), 1));
			if (trim($old_ext) != '' && $old_ext != $new_ext) {
				trigger_error('Can can not change the extension of the file. Upload a new file to change the extension.', E_USER_WARNING);
				$value = $old_filename;
				return false;
			}
			if (!is_file($this->data_path.'/'.$old_filename)) {
				trigger_error('Current file not found. Upload a file before renaming.', E_USER_WARNING);
				$value = $old_filename;
				return false;
			}
		}

		if (parent::setAttrValue($name, $value, $dont_run_updated) && $name == 'name' && !$uploading_file) {
			if (!@$this->saveWebPaths(Array($value))) {
				trigger_error('Failed updating web path. Path already exists?', E_USER_WARNING);
				parent::setAttrValue($name, $old_filename, $dont_run_updated);
				return false;
			}
			if (!rename($this->data_path.'/'.$old_filename, $this->data_path.'/'.$value)) {
				trigger_error('Failed renaming file, try again.', E_USER_WARNING);
				parent::setAttrValue($name, $old_filename, $dont_run_updated);
				return false;
			}
		}

		return true;

	}//end setAttrValue()


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
		$path = $this->data_path.'/'.$this->attr('name');
		if (is_file($path)) {
			$existing['path'] = $path;
			$existing['filename'] = $this->attr('name');
			$existing['modified'] = filectime($existing['path']);
			$existing['size'] = filesize($existing['path']);
			$size = getimagesize($path);
			$existing['width'] = $size[0];
			$existing['height'] = $size[1];
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
