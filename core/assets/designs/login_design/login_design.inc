<?php

require_once SQ_CORE_PACKAGE_PATH.'/designs/design/design.inc';

/**
* Login_Design
*
* Purpose
*
*
* @author  Blair Robertson <blair@squiz.net>
* @version $Version$ - 1.0
* @package Resolve_Packages
* @subpackage __core__
*/
class Login_Design extends Design
{
	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Login_Design($assetid=0)
	{
		$this->Design_Area($assetid);
	}//end constructor


	/**
	* Create this asset
	*
	* @param array()	&$link	information used to create the initial link
	*
	* @see Asset::create()
	* @return mixed int or false
	* @access public
	*/
	function create(&$link)
	{
		$db = &$GLOBALS['SQ_SYSTEM']->db;

		$sql = 'SELECT count(*) FROM sq_asset WHERE type_code = '.$db->quote($this->type());
		$exists = $db->getOne($sql);
		if (DB::isError($exists)) trigger_error($exists->getMessage().'<br/>'.$exists->getUserInfo(), E_USER_ERROR);

		if ($exists) {
			trigger_error('Unable to create another System Designs Folder, you are only allowed one', E_USER_WARNING);
			return false;
		}

		$this->setAttrValue('id_name', 'System Login Design');

		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		$linkid = parent::create($link);
		if ($linkid && $this->restoreLoginDesign()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
			return $linkid;

		} else {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;

		}// end if


	}//end create()


	/**
	* Restores/Resets the design for this to the installed defaults
	*
	* @return boolean
	* @access public
	*/
	function restoreLoginDesign() 
	{
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		if (!create_directory($this->data_path)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if (!create_directory($this->data_path_public)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$source_dir = dirname(__FILE__).'/design_files';
		if (!copy($source_dir.'/index.html', $this->data_path.'/parse.txt')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		if (!copy_directory($source_dir.'/files', $this->data_path_public.'/files')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		$edit_fns = $this->getEditFns();
		// First process the design
		if (!$edit_fns->parseAndProcessFile($this)) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// Now use it to generate the design file
		$this->generateDesignFile(false);

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}// end restoreLoginDesign()


	/**
	* Returns an array of all the permitted links type, the type asset and the cardinality
	*
	* @see Asset::_getAllowLinks()
	* @access private
	*/
	function _getAllowedLinks()
	{
		$ret_val = parent::_getAllowedLinks();
		// can't have any customisations
		unset($ret_val[SQ_LINK_TYPE_2]['design_customisation']);
		return $ret_val;
	}//end _getAllowedLinks()


}//end class
?>
