<?php
/**
* +--------------------------------------------------------------------+
* | Squiz.net Commercial Module Licence                                |
* +--------------------------------------------------------------------+
* | Copyright (c) Squiz Pty Ltd (ACN 084 670 600).                     |
* +--------------------------------------------------------------------+
* | This source file is not open source or freely usable and may be    |
* | used subject to, and only in accordance with, the Squiz Commercial |
* | Module Licence.                                                    |
* | Please refer to http://www.squiz.net/licence for more information. |
* +--------------------------------------------------------------------+
*
* $Id: cron_job_squid_cache_purge_edit_fns.inc,v 1.1.1.1 2006/10/05 05:37:23 bcaldwell Exp $
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
* @version $Revision: 1.1.1.1 $
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