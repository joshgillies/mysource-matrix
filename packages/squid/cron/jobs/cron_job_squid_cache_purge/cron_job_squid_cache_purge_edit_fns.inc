<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix Module file is Copyright (c) Squiz Pty Ltd    |
* | ACN 084 670 600                                                    |
* +--------------------------------------------------------------------+
* | IMPORTANT: This Module is not available under an open source       |
* | license and consequently distribution of this and any other files  |
* | that comprise this Module is prohibited. You may only use this     |
* | Module if you have the written consent of Squiz.                   |
* +--------------------------------------------------------------------+
*
* $Id: cron_job_squid_cache_purge_edit_fns.inc,v 1.2 2006/12/07 02:54:14 bcaldwell Exp $
*
*/

require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job_edit_fns.inc';

/**
* Cron_Job_Squid_Cache_Purge_Edit_Fns
*
* Purpose
*
*
* @author Rayn Ong <rong@squiz.net>
* @version $Revision: 1.2 $
* @package MySource_Matrix_Packages
* @subpackage squid
*/
class Cron_Job_Squid_Cache_Purge_Edit_Fns extends Cron_Job_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Cron_Job_Squid_Cache_Purge_Edit_Fns()
	{
		$this->Cron_Job_Edit_Fns();

	}//end constructor


	/**
	* Prints the tasks interface in a one line format
	*
	* @param object	&$asset					the owning asset
	* @param object	&$o						the backend outputter
	* @param string	$prefix					prefix for form elements
	* @param string	$when					prepared when string
	* @param string	$form_extra_elements	anything that goes into the 'extra' field
	*
	* @return boolean
	* @access public
	*/
	function paintInLineBackend(&$asset, &$o, $prefix, $when='', $form_extra_elements='')
	{
		$preposition = ($asset->attr('type') == 'one_off') ? translate('squid_clear_at') : translate('squid_clear_every');
		echo translate('squid_clear_this', $preposition);
		$this->_paintWhenBox($asset, $o, $prefix, $when, $form_extra_elements);

		return TRUE;

	}//end paintInLineBackend()


	/**
	* Process the tasks interface
	*
	* @param object	&$asset	the owning asset
	* @param object	&$o		the backend outputter
	* @param string	$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	function processInLineBackend(&$asset, &$o, $prefix)
	{
		$when = $this->_processWhenBox($asset, $o, $prefix);
		if ($when) {
			return $asset->setAttrValue('when', $when);
		} else {
			return FALSE;
		}

	}//end processInLineBackend()


}//end class

?>