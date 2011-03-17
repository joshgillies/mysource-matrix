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
* $Id: simple_edit_user.inc,v 1.2.2.3 2011/03/17 00:56:01 akarelia Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/users/user/user.inc';


/**
* Simple_Edit_User
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Revision: 1.2.2.3 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Simple_Edit_User extends User
{


	/**
	* Constructor
	*
	* @param int	$assetid	the asset id to be loaded
	*
	*/
	function Simple_Edit_User($assetid=0)
	{
		$this->User($assetid);

	}//end constructor


	/**
	* Check if this user can access the backend interface
	*
	* This will be disabled by default for a standard user, and enabled for
	* users who are allowed backend access (eg. backend user).
	*
	* @return boolean
	* @access public
	*/
	function canAccessBackend()
	{
		// Simple_Edit_User must not access the backend interface however it can access limbo
		// This function is called before limbo access is granted because traditionally, backend access is
		// required for limbo
		// Permit 'backend' access only if this user is requesting a limbo page
		// Allow simple edit user to have backend interface access when in cron
		// we also have to consider the EES/JS API. Here the State wouldnt be in SQ_IN_LIMBO
		// but never the less we are in simple edit. 
		// Lets be a bit chicky here and check the referer to decide if we are indeed in LIMBO
		// We also have to make sure that the user we are currently deling with isnt the user
		// who initiated the workflow. or else he deserves to be known
		$in_limbo = $workflow_starter = FALSE;
		if(isset($_SERVER['HTTP_REFERER']) && !empty($_SERVER['HTTP_REFERER'])) {
			$in_limbo = (strpos($_SERVER['HTTP_REFERER'],SQ_CONF_LIMBO_SUFFIX) !== FALSE) ? TRUE : FALSE;
		}

		// we are not in limbo or EES see if the user was a workflow starter and if the workflow has been canceled
		// see workflow_manager::cancelWorkflow()
		if(!$in_limbo && (isset($this->_tmp['starter_of_workflow']) && $this->_tmp['starter_of_workflow'])) $workflow_starter = TRUE;

		return (SQ_IN_LIMBO || SQ_IN_CRON || $in_limbo || $workflow_starter);

	}//end canAccessBackend()


}//end class
?>
