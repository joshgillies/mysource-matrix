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
* $Id: cron_job_expired_workflow_management.inc,v 1.1 2005/02/22 22:38:50 dbaranovskiy Exp $
*
*/


require_once SQ_INCLUDE_PATH.'/asset_management.inc';

/**
* Cron_Job_Expired_WorkFlow_Management
*
* @see Asset_Management
*
* @version $Revision: 1.1 $
* @package MySource_Matrix_Packages
* @subpackage __core__
*/
class Cron_Job_Expired_WorkFlow_Management extends Asset_Management
{

	/**
	* Constructor
	*
	*/
	function Cron_Job_Expired_WorkFlow_Management(&$pm)
	{
		$this->Asset_Management($pm);

		$this->vars['type']['default'] = 'repeating';
		$this->vars['when']['default'] = 'ET=';
		$this->vars['running_as']['default'] = $GLOBALS['SQ_SYSTEM']->am->getSystemAssetid('root_user');

	}//end constructor


	/**
	* Installs this asset
	*
	* @return boolean
	* @access public
	*/
	function _install()
	{
		if (!parent::_install()) return false;
		$cron_manager =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('cron_manager');
		$link =  Array ('asset'         => $cron_manager,
						'link_type'     => SQ_LINK_TYPE_2,
						'value'         => null,
						'sort_order'    => 0,
						'is_dependant'  => 0,
						'is_exclusive'  => 1,
						);
		$job = new Cron_Job_Expired_WorkFlow();
		$job->create($link);
		return true;

	}

}//end class
?>
