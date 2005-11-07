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
* $Id: bulkmail_job_edit_fns.inc,v 1.10 2005/11/07 04:46:35 rong Exp $
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
* @version $Revision: 1.10 $
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
	* Paints the header detail checkbox, whether to use the post office header or not
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintHeaderDetailsCheckBox(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		$use_post_office_header = $asset->attr('use_post_office_header');

		// paint checkbox
		if ($write_access) {
			check_box($prefix.'_use_post_office_header', true, $use_post_office_header);
		} else {
			?>
				<img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $use_post_office_header ? 'tick' : 'cross'; ?>.gif" width="15" height="15" />
			<?php
		}
/*
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
*/
		return $write_access;

	}//end paintHeaderDetailsCheckBox()


	/**
	* Process the header detail checkbox
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processHeaderDetailsCheckBox(&$asset, &$o, $prefix)
	{
		// once live, header details are not changeable
		if ($asset->status == SQ_STATUS_LIVE) return false;
		if (!$asset->writeAccess('attributes')) return false;

		// save whether to use post office header details
		$use_header_details = isset($_POST[$prefix.'_use_post_office_header']);
		$asset->setAttrValue('use_post_office_header', $use_header_details);

		return true;

	}//end processHeaderDetailsCheckBox()


	/**
	* Check if we should use the post office header
	*
	* @param object	&$asset	the asset to which we belong
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function showJobHeader(&$asset, $prefix)
	{
		return !($asset->attr('use_post_office_header'));

	}//end showJobHeader()


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