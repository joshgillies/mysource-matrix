<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Open Source Licence                                      |
* +--------------------------------------------------------------------+
* | Copyright (c), 2003 Squiz Pty Ltd (ABN 77 084 670 600).            |
* +--------------------------------------------------------------------+
* | This source file may be used subject to, and only in accordance    |
* | with, the Squiz Open Source Licence Agreement found at             |
* | http://www.squiz.net/licence.                                      |
* | Make sure you have read and accept the terms of that licence,      |
* | including its limitations of liability and disclaimers, before     |
* | using this software in any way. Your use of this software is       |
* | deemed to constitute agreement to be bound by that licence. If you |
* | modify, adapt or enhance this software, you agree to assign your   |
* | intellectual property rights in the modification, adaptation and   |
* | enhancement to Squiz Pty Ltd for use and distribution under that   |
* | licence.                                                           |
* +--------------------------------------------------------------------+
*
* $Id: package_manager_core.inc,v 1.2.2.1 2004/02/18 11:39:04 brobertson Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_INCLUDE_PATH.'/package_manager.inc';

/**
* Package_Manager_Core
*
* Purpose
*    Manages the install and upgrade of the core package and core assets, 
*    uses info gleened from the package.xml and asset.xml files
*
* @author  Blair Robertson <brobertson@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Package_Manager_Core extends Package_Manager
{

	/**
	* @var array
	* Holds all the searchable components that ALL asset's should have
	*
	*/
	var $default_components = Array(
									'__name__' => Array (
										'description'		=> 'Asset Name',
										'default_weighting'	=> '-1',
									),
								);


	/**
	* Constructor
	*
	*/
	function Package_Manager_Core()
	{
		$this->_full_path = dirname(__FILE__);
		$this->Package_Manager();

	}//end constructor

	/**
	* Returns the name of the package that this management class is working for
	*
	* @access public
	*/
	function getPackageName()
	{
		// need to override because we don't really want to have to name the Package_Manager___Core__ :)
		return '__core__';

	}//end getPackageName()




	/**
	* Installs/Updates the passed package in the system
	*
	* @access public
	*/
	function updatePackageDetails()
	{
		if (!$this->checkPackageDetails()) {
			return false;
		}

		// open the transaction
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		//// Before we update any of the asset types we are going to make sure that 
		//// the default search components are in place
		$db =& $GLOBALS['SQ_SYSTEM']->db;

		// clear all weightings for this type_code
		$sql = 'DELETE FROM '. SQ_TABLE_PREFIX .'asset_indexing_weightings_default
				WHERE type_code = '.$db->quote('asset');

		$result = $db->query($sql);
		if (DB::isError($result)) {
			// Abort Transaction
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
			return false;
		}

		foreach ($this->default_components as $component => $weighting) {
			$sql = 'INSERT INTO '. SQ_TABLE_PREFIX .'asset_indexing_weightings_default
					(type_code, component, description, weighting)
					VALUES ('.$db->quote('asset').', '.$db->quote($component).',
					'.$db->quote($weighting['description']).','.$db->quote($weighting['default_weighting']).')';

			$result = $db->query($sql);
			if (DB::isError($result)) {
				// Abort Transaction
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				trigger_error($result->getMessage().'<br/>'.$result->getUserInfo(), E_USER_WARNING);
				return false;
			}
		}// end foreach


		// Now we can do the normal package update 
		if (!parent::updatePackageDetails()) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			return false;
		}

		// Commit the transaction if we got this far
		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		return true;

	}//end updatePackageDetails()

}//end class

?>