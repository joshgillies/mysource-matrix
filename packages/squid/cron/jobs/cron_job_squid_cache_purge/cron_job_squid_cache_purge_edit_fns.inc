<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd	   |
* | ACN 084 670 600													   |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.														   |
* +--------------------------------------------------------------------+
*
* $Id: cron_job_squid_cache_purge_edit_fns.inc,v 1.5 2012/08/30 00:59:22 ewang Exp $
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
* @version $Revision: 1.5 $
* @package MySource_Matrix_Packages
* @subpackage squid
*/
class Cron_Job_Squid_Cache_Purge_Edit_Fns extends Cron_Job_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Prints the tasks interface in a one line format
	*
	* @param Asset				$asset					the owning asset
	* @param Backend_Outputter	$o						the backend outputter
	* @param string				$prefix					prefix for form elements
	* @param string				$when					prepared when string
	* @param string				$form_extra_elements	anything that goes into the 'extra' field
	*
	* @return boolean
	* @access public
	*/
	public function paintInLineBackend(Asset $asset, Backend_Outputter $o, $prefix, $when='', $form_extra_elements='')
	{
		$preposition = ($asset->attr('type') == 'one_off') ? translate('squid_clear_at') : translate('squid_clear_every');
		echo sprintf(translate('squid_clear_this'), $preposition);
		$this->_paintWhenBox($asset, $o, $prefix, $when, $form_extra_elements);

		return TRUE;

	}//end paintInLineBackend()


	/**
	* Process the tasks interface
	*
	* @param Asset				$asset	the owning asset
	* @param Backend_Outputter	$o		the backend outputter
	* @param string				$prefix	prefix for form elements
	*
	* @return boolean
	* @access public
	*/
	public function processInLineBackend(Asset $asset, Backend_Outputter $o, $prefix)
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