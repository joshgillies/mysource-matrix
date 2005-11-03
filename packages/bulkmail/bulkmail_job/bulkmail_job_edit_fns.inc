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
* $Id: bulkmail_job_edit_fns.inc,v 1.9 2005/11/03 21:53:23 rong Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/bulkmail/bulkmail_post_office/bulkmail_post_office_edit_fns.inc';
require_once SQ_LIB_PATH.'/html_form/html_form.inc';


/**
* Bulkmail_Job_Edit_Fns
*
* Purpose
*
*
* @author  Nathan de Vries <ndvries@squiz.net>
* @author  Rayn Ong <rong@squiz.net>
*
* @version $Revision: 1.9 $
* @package MySource_Matrix_Packages
* @subpackage bulkmail
*/
class Bulkmail_Job_Edit_Fns extends Bulkmail_Post_Office_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Bulkmail_Job_Edit_Fns()
	{
		$this->Asset_Edit_Fns();
		unset($this->static_screens['preview']);

	}//end constructor


//--        DETAILS        --//


	/**
	* Paints the header details interface for the bmail
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintHeaderDetails(&$asset, &$o, $prefix)
	{
		$width = 60;
		$write_access = $asset->writeAccess('attributes');
		$post_office =& $asset->getPostOffice();
		$details_setting = $asset->attr('details_setting');

		if (!is_null($post_office)) {
			// if checkbox is checked, use post office's header details
			$checkbox_enabled = '';
			$field_note = translate('bulkmail_header_detail_check_enabled');
			$hide_header =  array_get_index($details_setting, 'use_post_office_header', false);
		} else {
			// no inheritance otherwise
			$checkbox_enabled = 'disabled="true"';
			$field_note = translate('bulkmail_header_detail_check_disabled');
			$hide_header = false;
		}

		// paint checkbox, checkbox is not enabled unless job is linked under a post office
		$field_name = translate('bulkmail_header_detail_check_field_name');
		$o->openField($field_name, '', $field_note);
			if ($write_access) {
				check_box($prefix.'_use_post_office_header', true, $hide_header, 'hideHeader(this.checked)', $checkbox_enabled);
			} else {
				?>
					<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $hide_header ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
				<?php
			}
		$o->closeField();

		// javascript to disable input fields when the checkbox is checked
		?>
		<script language="JavaScript" type="text/javascript">
			function hideHeader(checked) {
				var prefix = '<?php echo $prefix; ?>';
				var regex_string1 = prefix + '_header_detail';
				var re1 = new RegExp(regex_string1);
				var elements = document.getElementsByTagName('tr');
				for (i = 0; i < elements.length; i++) {
					if (elements[i].id.match(re1)) {
						elements[i].style['display'] = checked ? 'none' : '';
					}
				}
			}
		</script>
		<?php

		// paint the actual header details
		return parent::paintHeaderDetails($asset, $o, $prefix, $hide_header);

	}//end paintHeaderDetails()


	/**
	* Process the header details for the bmail
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processHeaderDetails(&$asset, &$o, $prefix)
	{
		// once live, header details are not changeable
		if ($asset->status == SQ_STATUS_LIVE) return false;
		if (!$asset->writeAccess('attributes')) return false;

		// save whether to use post office header details
		$use_header_details = isset($_POST[$prefix.'_use_post_office_header']);
		$details_setting = $asset->attr('details_setting');
		$details_setting['use_post_office_header'] = $use_header_details ? true : false;
		$asset->setAttrValue('details_setting', $details_setting);

		// only process this if not inherit from parent
		if (!$use_header_details) {
			return parent::processHeaderDetails($asset, $o, $prefix);
		} else {
			return true;
		}

	}//end processHeaderDetails()


//--        JOB MANAGEMENT        --//


	/**
	* Paint the job queue
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintJobQueue(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$bulkmail_manager =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$bulkmail_manager_edit_fns = $bulkmail_manager->getEditFns();
		$o->openField('');
			$jobs = $bulkmail_manager->getQueuedJobs($asset->id);
			$bulkmail_manager_edit_fns->_paintJobQueue($asset, $jobs);
		$o->closeField();

		return $write_access;

	}//end printJobQueue()


	/**
	* Process the job queue
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processJobQueue(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$bulkmail_manager =& $GLOBALS['SQ_SYSTEM']->am->getSystemAsset('bulkmail_manager');
		$bulkmail_manager_edit_fns = $bulkmail_manager->getEditFns();
		if (!$write_access) return;

		$jobs = $bulkmail_manager->getQueuedJobs($asset->id);
		$bulkmail_manager_edit_fns->_processJobQueue($asset, $jobs);

		return true;

	}//end processJobQueue()


}//end class
?>