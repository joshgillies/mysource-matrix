<?php

require_once SQ_INCLUDE_PATH.'/asset.inc';

/**
* Folder
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
*/
class Folder extends Asset
{

	/**
	* Constructor
	*
	* @param int    $assetid      the asset id to be loaded
	*
	*/
	function Folder($assetid=0) 
	{
		$this->Asset($assetid);
	}


	/*
	* Create this asset
	*
	* @param string $name   the name of this folder
	*
	* @return boolean
	* @access public
	*/
	function create($name)
	{
		$name = trim($name);

		if ($name == '') {
			trigger_error('Unable to create a Folder with a blank name', E_USER_WARNING);
			return false;
		}

		if (parent::create()) {

			$this->setAttribute('name',   $name);
			return true;

		}#end if

		return false;

	}#end create()

	/*
	* Returns name of the asset
	*
	* @ see Asset::_getName()
	* @access public
	*/
	function _getName() {
		return $this->attr('name');
	}#end _getName();


	/*
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks() {

		return Array(SQ_LINK_EXCLUSIVE => Array('asset' => 'M'),
					 SQ_LINK_UNITE     => Array('asset' => 'M'),
					 SQ_LINK_NOTICE    => Array('asset' => 'M')
					);

	}#end _getAllowedLinks()


	/*
	* handles the creation of this asset, including painting 
	*
	* @param object Backend $backend
	*
	* @access public
	*/
	function printBackend(&$backend) {

		$o = &$backend->out;

		$o->setHeading('New Folder', 'create');
		$o->addHiddenField('asset_action', (($this->id) ? 'edit' : 'create'));

		$o->openSection('Create Settings');
		$o->openField('Folder Name');
		$o->text_box('name', '', 10);

		pre_echo($_GET);
		pre_echo($_POST);

		$o->commitButton('Create');

	}//end printBackend()

	/*
	* processes a backend submission from this asset, returns true if all OK
	*
	* @ return boolean
	* @access public
	*/
	function processBackend() {

		switch($_POST['asset_action']) {
			case 'create' :
				return $this->create($_POST['name']);
			break;

			default :
				return false;



		}// end switch

	}// end processBackend()



}#end class
?>
