<?php
/**
* +--------------------------------------------------------------------+
* | This MySource Matrix CMS file is Copyright (c) Squiz Pty Ltd       |
* | ABN 77 084 670 600                                                 |
* +--------------------------------------------------------------------+
* | IMPORTANT: Your use of this Software is subject to the terms of    |
* | the Licence provided in the file licence.txt. If you cannot find   |
* | this file please contact Squiz (www.squiz.com.au) so we may provide|
* | you a copy.                                                        |
* +--------------------------------------------------------------------+
*
* $Id: cron_job_sync_cmis_bridges_edit_fns.inc,v 1.2 2012/08/30 00:59:19 ewang Exp $
*
*/


require_once SQ_CORE_PACKAGE_PATH.'/system/cron/cron_job/cron_job_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';

/**
* Cron_Job_Sync_CMIS_Bridges_Edit_Fns
*
*
* @version $Revision: 1.2 $
* @author  Mohamed Haidar <mhaidar@squiz.com.au>
* @package MySource_Matrix_Packages
* @subpackage sharepoint
*/
class Cron_Job_Sync_CMIS_Bridges_Edit_Fns extends Cron_Job_Edit_Fns
{


	/**
	* Constructor
	*
	* @see Asset_Edit_Fns::__construct()
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor
	
	
	/**
	* paintScheduleInterface.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintScheduleInterface(&$asset, &$o, $prefix)
	{
		//default values
		$dl_check= false;
		$hr_check = false;
		$hr_mins = '00';
		$dl_hours = '00';
		$dl_mins = '00';
		
		//create minutes array
		$mins = array();
		for($i=0;$i<60;$i++) {
			if($i<10) {
				$mins['0'.$i] = '0'.$i;
			} else {
				$mins[$i] = $i;
			}
		}
		
		//create hours array
		$hours = array();
		for($i=0;$i<24;$i++) {
			if($i<10) {
				$hours['0'.$i] = '0'.$i;
			} else {
				$hours[$i] = $i;
			}
		}
		$when = $asset->attr('when');

		$when_array = split('=', $when);

		if(is_array($when_array) && count($when_array)==2) {
			if($when_array[0]=='HR') {
				$hr_check = true;
				$hr_mins = $when_array[1];
			} else {
				$dl_check= true;
				$time = split(':',$when_array[1]);
				if(is_array($time) && count($time)==2) {
					$dl_mins = $time[1];
					$dl_hours = $time[0];
				}
			}
		}

        if ($asset->writeAccess('attributes')) {
			?>
			<div>
				<table style="width:auto;border-spacing:0px;" border="0">
					<tr>
						<td><span style="font-weight:bold;">Hourly</span></td>
						<td><?php radio_button($prefix.'_schedule','HR', $hr_check); ?></td>
						<td><?php combo_box($prefix.'_h_mins',$mins, false, $hr_mins); ?> min of every hour</td>
					</tr>
					<tr>
						<td><span style="font-weight:bold;">Daily</span></td>
						<td><?php radio_button($prefix.'_schedule', 'DL', $dl_check); ?></td>
						<td><?php combo_box($prefix.'_d_hours',$hours, false, $dl_hours); ?>:<?php combo_box($prefix.'_d_mins',$mins, false, $dl_mins); ?> on every day</td>
					</tr>
				</table>
			</div>
            <?php
		} else {
			if($hr_check) {
			?>
			<div>
				<table style="width:auto;border-spacing:0px;" border="0">
					<tr>
						<td>CMIS Bridge Files Will be Updated Hourly at </td>
						<td><?php echo $hr_mins;?> min</td>
					</tr>
				</table>
			</div>
			<?php
			} else {
			?>
			<div>
				<table style="width:auto;border-spacing:0px;" border="0">
					<tr>
						<td>CMIS Bridge Files Will be Updated Daily at </td>
						<td><?php echo $dl_hours;?>:<?php echo $dl_mins;?></td>
					</tr>
				</table>
			</div>
			<?php
			}
		}
		return TRUE;
	}
	
	
	/**
	* processScheduleInterface.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processScheduleInterface(&$asset, &$o, $prefix)
	{
		if(isset($_POST[$prefix.'_schedule'])) {
			$_schedule = $_POST[$prefix.'_schedule'];
		}
		if(isset($_POST[$prefix.'_h_mins'])) {
			$_h_mins = $_POST[$prefix.'_h_mins'];
		}
		if(isset($_POST[$prefix.'_d_hours'])) {
			$_d_hours = $_POST[$prefix.'_d_hours'];
		}
		if(isset($_POST[$prefix.'_d_mins'])) {
			$_d_mins = $_POST[$prefix.'_d_mins'];
		}
		
		if($_schedule=='HR') {
			$when = $_schedule.'='.$_h_mins;
		} else {
			$when = $_schedule.'='.$_d_hours.':'.$_d_mins;	
		}
		
		if ($when) {
            return $asset->setAttrValue('when', $when);
        } else {
            return FALSE;
        }
	}


}//end class

?>