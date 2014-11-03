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
* $Id: page_online_quiz_edit_fns.inc,v 1.14 2012/08/30 00:58:09 ewang Exp $
*
*/


require_once SQ_PACKAGES_PATH.'/cms/page_templates/page_asset_listing/page_asset_listing_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';
require_once SQ_FUDGE_PATH.'/general/datetime.inc';


/**
* Page_Online_Quiz_Edit_Fns
*
* Edit Functions for the Online Quiz
* Includes 'Submission Logs' handling customised from 'Form Email' implementation
*
*
* @author  Luke Wright <lwright@squiz.net>
* @author  Mark Brydon <mbrydon@squiz.net>
* @version $Revision: 1.14 $
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Page_Online_Quiz_Edit_Fns extends Page_Asset_Listing_Edit_Fns
{


	/**
	* Constructor
	*
	* @return void
	* @access public
	*/
	function __construct()
	{
		parent::__construct();

	}//end constructor


	/**
	* Paint the interface for handling the 'user_details_source' attribute
	* and toggling the 'user_details' bodycopy and form
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintUserDetailsSource(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		$attr = $asset->getAttribute('user_details_source');
		$attr->paint($prefix.'_'.$attr->id, !$write_access);

		$form = $asset->getForm();
		if (!is_null($form)) {
			echo '<p>'.get_asset_tag_line($form->id, 'details').'</p>';
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($form);

		$bodycopy = $asset->getBodycopy('user_details');
		if (!is_null($bodycopy)) {
			echo '<p>'.get_asset_tag_line($bodycopy->id, 'contents').'</p>';
		}
		$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bodycopy);

		return $write_access;

	}//end paintUserDetailsSource()


	/**
	* Process the interface for handling the 'user_details_source' attribute
	* and toggling the 'user_details' bodycopy and form
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processUserDetailsSource(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if ($write_access) {
			$attr = $asset->getAttribute('user_details_source');
			$attr->process($prefix.'_'.$attr->id);
			if (!$attr->processed || !$asset->setAttrValue($attr->name, $attr->value)) {
				// bail now if we can't process the attribute properly
				return FALSE;
			}
		}

		return $write_access;

	}//end processUserDetailsSource()


	/**
	* Paint the interface for handling the 'after_submit_action' attribute
	* and toggling the 'thank_you' bodycopy
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAfterSubmitAction(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes') && $asset->writeAccess('links');

		$attr = $asset->getAttribute('after_submit_action');
		$attr->paint($prefix.'_'.$attr->id, !$write_access);

		if ($attr->value == 'thank_you') {
			$bodycopy = $asset->getBodycopy('thank_you');
			if (!is_null($bodycopy)) {
				echo '<p>'.get_asset_tag_line($bodycopy->id, 'contents').'</p>';
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bodycopy);
		}

		return $write_access;

	}//end paintAfterSubmitAction()


	/**
	* Process the interface for handling the 'after_submit_action' attribute
	* and toggling the 'thank_you' bodycopy
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAfterSubmitAction(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes') && $asset->writeAccess('links');

		if ($write_access) {
			$attr = $asset->getAttribute('after_submit_action');
			$attr->process($prefix.'_'.$attr->id);
			if (!$attr->processed || !$asset->setAttrValue($attr->name, $attr->value)) {
				// bail now if we can't process the attribute properly
				return FALSE;
			}

			// if form is selected, set up the thank_you bodycopy
			$enable_bodycopy = ($attr->value == 'thank_you');
			$asset->setBodycopy('thank_you', $enable_bodycopy);
		}

		return $write_access;

	}//end processAfterSubmitAction()


	/**
	* Process the interface for handling the 'after_submit_action' attribute
	* and toggling the 'thank_you' bodycopy
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processCanRetake(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes') && $asset->writeAccess('links');

		if ($write_access) {
			$attr = $asset->getAttribute('can_retake');
			$attr->process($prefix.'_'.$attr->id);
			if (!$attr->processed || !$asset->setAttrValue($attr->name, $attr->value)) {
				// bail now if we can't process the attribute properly
				return FALSE;
			}

			// if form is selected, set up the thank_you bodycopy
			// can_retake == TRUE means we don't need the "Already Taken" bodycopy
			$enable_bodycopy = !($attr->value);
			$asset->setBodycopy('already_taken', $enable_bodycopy);
		}

		return $write_access;

	}//end processCanRetake()


	/**
	* Paint the interface for handling the 'can_retake' attribute
	* and toggling the'already_taken' bodycopy
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCanRetake(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes') && $asset->writeAccess('links');

		$attr = $asset->getAttribute('can_retake');
		$attr->setEditParam('true_text', translate('Yes'));

		$attr->setEditParam('false_text', translate('No'));

		$attr->paint($prefix.'_'.$attr->id, !$write_access);

		if ($attr->value == FALSE) {
			$bodycopy = $asset->getBodycopy('already_taken');
			if (!is_null($bodycopy)) {
				echo '<p>'.get_asset_tag_line($bodycopy->id, 'contents').'</p>';
			}
			$GLOBALS['SQ_SYSTEM']->am->forgetAsset($bodycopy);
		}

		return $write_access;

	}//end paintCanRetake()


	/**
	* Paint links to the default formats
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDefaultFormatLinks(&$asset, &$o, $prefix)
	{
		?>
		<table class="sq-backend-table">
			<tr>
				<th><?php echo translate('Edit Format') ?></th>
				<th><?php echo translate('Note') ?></th>
			</tr>
			<?php
			$bodycopies = $asset->_bodycopies;

			// manually print page_contents and no_results
			unset($bodycopies['page_contents']);
			unset($bodycopies['no_results']);
			$this->_paintDefaultFormatLink($asset->id, 'page_contents', translate('This format defines the layout of the page. Ensure you use the %asset_listing% keyword to include the list of assets on the page.'));
			$this->_paintDefaultFormatLink($asset->id, 'no_results', translate('This format defines the layout of the page when the <b>Customise No Results Bodycopy</b> option is turned on, and there are no assets to list.'));

			$bodycopy_descs = Array(
				'thank_you'     => translate('This format defines the layout of the "Thank You" page that (optionally) appears when the quiz is completed.'),
				'already_taken' => translate('This format defines the layout of the "Already Taken" notification page appears (which displays when the test has already been taken).'),
				'user_details'  => translate('This format defines the layout of the "User Details" page that appears when a user has to enter their details. Make sure to include the %form_contents% keyword.'),
			);
			
			foreach ($bodycopies as $bodycopy_code => $bodycopy_name) {
				$this->_paintDefaultFormatLink($asset->id, $bodycopy_code, $bodycopy_descs[$bodycopy_code]);
			}

			$this->_paintDefaultFormatLink($asset->id, 'user_details', translate('This form defines which questions can/must be filled out before the quiz can be taken. Note that this form will be displayed to non-logged-in users regardless of the "User Details Source" setting.'), 'simple_form', 'details');

			$folder = $asset->getFolder('type_formats');
			$this->_paintDefaultFormatLink($folder->id, 'default_format', translate('This is the default format for all assets in the list if they do not have a customised format for their asset type or position.'));

			?>
		</table>
		<?php

		return FALSE;

	}//end paintDefaultFormatLinks()


	/**
	* Paints the download date range selector for the export block
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDownloadDateRange(&$asset, &$o, $prefix)
	{
		$parameters = Array(
						'min'			=> '2003-01-01 00:00:00',
						'max'			=> ts_iso8601(time()),
						'allow_circa'	=> '0',
						'show'			=> Array('y', 'm', 'd'),
						'null'			=> Array(),
						'style'			=> Array(
											'y'	=> 's',
											'm'	=> 's',
											'd'	=> 's',
										   ),
					  );

		$parameters['print_format'] = 'j M Y';

		$from_value = isset($_POST[$prefix.'_export_fromvalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_export_fromvalue']['m'],$_POST[$prefix.'_export_fromvalue']['d'],$_POST[$prefix.'_export_fromvalue']['y'])) : date('Y-m-d', time()-7*86400).' --:--:--';
		$to_value = isset($_POST[$prefix.'_export_tovalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_export_tovalue']['m'],$_POST[$prefix.'_export_tovalue']['d'],$_POST[$prefix.'_export_tovalue']['y'])) : date('Y-m-d').' --:--:--';

		$from = new Datetime_Field($prefix.'_export_from', $from_value, $parameters);
		$to = new Datetime_Field($prefix.'_export_to', $to_value, $parameters);

		$from->printField();
		echo ' '.translate('To').' ';

		$to->printField();

	}//end paintDownloadDateRange()


	/**
	* Paints the download format selector for the export block
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDownloadFormat(&$asset, &$o, $prefix)
	{
		$options = Array(
					''			=> '------- '.translate('Please Select').' -------',

					'csv'		=> translate('Comma Separated Values (CSV)'),

					'csv_nhr'	=> translate('Comma Separated Values (CSV) - No Header Row'),

				   );

		echo combo_box($prefix.'_export_format', $options, FALSE, '');
		echo hidden_field($prefix.'_export_switch', '0');

		$delimiter_text = isset($_POST[$prefix.'_delimiter']) ? $_POST[$prefix.'_delimiter'] : ',';
		echo ' &nbsp; Delimiter &nbsp;';
		echo text_box($prefix.'_delimiter',$delimiter_text,1,1,TRUE);

	}//end paintDownloadFormat()


	/**
	* Paints the fields available for export
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDownloadFields(&$asset, &$o, $prefix)
	{
		// Grab all available questions
		$user_details_form = $asset->getForm();
		$user_details_form_questions = $user_details_form->getQuestions();
		$quiz_questions = $asset->getQuestions(TRUE, TRUE);

		echo check_box($prefix.'_export_fields_select_all', '1', '', 'sq_set_check_all(this.form, "'.$prefix.'_export_fields[]", this.checked);').'<b>'.translate('Select All').'</b>';

		echo '<br><br>';

		echo '<p><b>Submission Details</b><br>';
		echo check_box($prefix.'_export_fields[]', '__time__', '1').translate('Submission Time').'<br/>';

		echo check_box($prefix.'_export_fields[]', '__user__', '1').translate('User ID').'<br/>';

		echo check_box($prefix.'_export_fields[]', '__ipaddress__', '1').translate('IP Address').'<br/>';

		echo check_box($prefix.'_export_fields[]', '__score__', '1').translate('Score').'<br/>';

		echo '</p>';


		if (!empty($user_details_form_questions)) {
			echo '<p><b>User Details Form Questions</b><br>';
			foreach ($user_details_form_questions as $user_details_form_question_assetid => $user_details_form_question_details) {
				$user_details_form_question = $GLOBALS['SQ_SYSTEM']->am->getAsset($user_details_form_question_assetid);
				$user_details_form_questions[$user_details_form_question_assetid] = $user_details_form_question->getVal('title');

				echo check_box($prefix.'_export_fields[]', $user_details_form_question_assetid, '1');
				echo $user_details_form_questions[$user_details_form_question_assetid].'<br/>';

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($user_details_form_question);
			}
			echo '</p>';
		}

		if (!empty($quiz_questions)) {
			echo '<p><b>Quiz Questions</b><br>';
			$q_num = 1;
			foreach ($quiz_questions as $quiz_question_assetid => $quiz_question_details) {
				$quiz_question = $GLOBALS['SQ_SYSTEM']->am->getAsset($quiz_question_assetid);
				$quiz_questions[$quiz_question_assetid] = trim(strip_tags($quiz_question->getQuestionText()));

				echo check_box($prefix.'_export_fields[]', $quiz_question_assetid, '1');
				echo 'Q'.$q_num.'. '.$quiz_questions[$quiz_question_assetid].'<br/>';
				$q_num++;

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($quiz_question);
			}
			echo '</p>';
		}

	}//end paintDownloadFields()


	/**
	* Paints the download button for the export block
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintDownloadButton(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) {
			$o->note(translate('Please acquire the lock before exporting.'));

			return FALSE;
		}

		// explanation for onClick for the Download button, there is some trickery involved here >:)
		// - set export switch to 1 = "ON"
		// - submit form (automatically sets SQ_FORM_SUBMITTED) - we're still here though, as the
		//   browser will be sent through the CSV/XML file and it will be offered to save - not display
		//   (DEPENDS on browser - CSV should be offered to save, XML could be intercepted by browser!)
		// - unset SQ_FORM_SUBMITTED so the form can still be submitted via Commit
		// - set export switch to 0 = "OFF" so Commit doesn't try to save a file either
		// - return true for the event as per JS standard

		echo normal_button($prefix.'_export', translate('Download File'), 'this.form.'.$prefix.'_export_switch.value = 1; this.form.submit(); SQ_FORM_SUBMITTED = false; this.form.'.$prefix.'_export_switch.value = 0; return true;');


	}//end paintDownloadButton()


	/**
	* Paints the date range selector
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSubmissionDateRange(&$asset, &$o, $prefix)
	{
		$parameters = Array(
						'min'			=> '2003-01-01 00:00:00',
						'max'			=> ts_iso8601(time()),
						'allow_circa'	=> '0',
						'show'			=> Array('y', 'm', 'd'),
						'null'			=> Array(),
						'style'			=> Array(
											'y'	=> 's',
											'm'	=> 's',
											'd'	=> 's',
										   ),
					  );

		$parameters['print_format'] = 'j M Y';

		$from_value = isset($_POST[$prefix.'_fromvalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_fromvalue']['m'],$_POST[$prefix.'_fromvalue']['d'],$_POST[$prefix.'_fromvalue']['y'])) : date('Y-m-d',time()-7*86400).' --:--:--';
		$to_value = isset($_POST[$prefix.'_tovalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_tovalue']['m'],$_POST[$prefix.'_tovalue']['d'],$_POST[$prefix.'_tovalue']['y'])) : date('Y-m-d').' --:--:--';

		$from = new Datetime_Field($prefix.'_from', $from_value, $parameters);
		$to = new Datetime_Field($prefix.'_to', $to_value, $parameters);

		$from->printField();
		echo ' '.translate('To').' ';

		$to->printField();

		return TRUE;

	}//end paintSubmissionDateRange()


	/**
	* Paints the submission count interface
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSubmissionCount(&$asset, &$o, $prefix)
	{
		$num_per_page = array_get_index($_REQUEST, $prefix.'_per_page', 50);
		if ($num_per_page < 1) $num_per_page = 50;

		text_box($prefix.'_per_page', $num_per_page, 3);

	}//end paintSubmissionCount()


	/**
	* Paints the submission list
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSubmissionList(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('links');
		$expand_list = isset($_POST[$prefix.'_expand']) ? $_POST[$prefix.'_expand'] : Array();
		$expand_sections = isset($_POST[$prefix.'_expand_sections']) ? $_POST[$prefix.'_expand_sections'] : Array();
		$num_subs = 0;

		// grab the number of assets per page, defaulting to 50 at a time
		$num_per_page = array_get_index($_REQUEST, $prefix.'_per_page', 50);
		if ($num_per_page < 1) $num_per_page = 50;

		$submission_offset = array_get_index($_REQUEST, $prefix.'_submission_offset', 0);

		$from_value = isset($_POST[$prefix.'_fromvalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_fromvalue']['m'],$_POST[$prefix.'_fromvalue']['d'],$_POST[$prefix.'_fromvalue']['y'])) : date('Y-m-d',time()-7*86400).' --:--:--';
		$to_value = isset($_POST[$prefix.'_tovalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_tovalue']['m'],$_POST[$prefix.'_tovalue']['d'],$_POST[$prefix.'_tovalue']['y'])) : date('Y-m-d').' --:--:--';

		$from_value = iso8601_date_component($from_value).' 00:00:00';
		$to_value = iso8601_date_component($to_value).' 23:59:59';

		// convert the ISO times to time stamps but manually because we can't be sure
		// about the h:m:s section of the above times
		$from_ts = mktime(0,0,0,$from_value[1],$from_value[2],$from_value[0]);
		$to_ts   = mktime(23,59,59,$to_value[1],$to_value[2],$to_value[0]);

		$sql = 'SELECT
					a.assetid, a.created_userid, a.created
				FROM
					('.SQ_TABLE_RUNNING_PREFIX.'ast a
					JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_typ_inhd i ON a.type_code = i.type_code)
					JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l
					ON l.minorid = a.assetid';

		$count_sql = 'SELECT
						count(*)
					  FROM
						('.SQ_TABLE_RUNNING_PREFIX.'ast a
						JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_typ_inhd i ON a.type_code = i.type_code)
						JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l
						ON l.minorid = a.assetid';

		$sub_folder = $asset->getSubmissionsFolder();

		$db = Dal::getDb();

		$ts_from_fragment = db_extras_todate(MatrixDAL::getDbType(), ':from_date', FALSE);
		$ts_to_fragment   = db_extras_todate(MatrixDAL::getDbType(), ':to_date',   FALSE);

		$where = 'l.majorid IN (:assetid, :subfolder_assetid)
					AND i.inhd_type_code = :inhd_type_code
					AND a.created BETWEEN '.$ts_from_fragment.'
					AND '.$ts_to_fragment;

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'i');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');

		// grab the number of assets
		$asset_count = 0;
		try {
			$query = MatrixDAL::preparePdoQuery($count_sql.$where);
			MatrixDAL::bindValueToPdo($query, 'assetid',           $asset->id);
			MatrixDAL::bindValueToPdo($query, 'subfolder_assetid', $sub_folder->id);
			MatrixDAL::bindValueToPdo($query, 'inhd_type_code',    'online_quiz_submission');
			MatrixDAL::bindValueToPdo($query, 'from_date',         $from_value);
			MatrixDAL::bindValueToPdo($query, 'to_date',           $to_value);
			$asset_count = MatrixDAL::executePdoOne($query);
		} catch (Exception $e) {
			throw new Exception($e.getMessage());
		}

		$num_pages = ceil($asset_count / $num_per_page);


		if (($submission_offset % $num_per_page) != 0) {
			$submission_offset = $submission_offset - ($submission_offset % $num_per_page);
		}

		if ($submission_offset < 0) $submission_offset = 0;

		if ($submission_offset >= $asset_count) {
			$submission_offset = ($num_pages - 1) * $num_per_page;
		}

		$sql = $sql.$where.' ORDER BY a.created DESC';

		$sql = db_extras_modify_limit_clause($sql, MatrixDAL::getDbType(),$num_per_page, $submission_offset);
		if ($asset_count == 0) {
			?><p><i><?php echo translate('There are no submissions during the time period specified above') ?></i></p><?php

		} else {
			?><style type="text/css">
				td.sq-form-sub {
					padding-top: 0px;
				}
				.sq-form-sub table.sq-backend-table {
					border-top: 0px;
					margin: 0 8px 8px 8px;
				}
				.sq-form-sub table.sq-backend-table th {
					font-weight: normal;
					text-decoration: italic;
				}
				table.sq-backend-table tr td.sq-form-sub-header {
					background-color: #666;
					color: #fff;
					font-size: 11px;
				}
			</style>
			<script type="text/javascript"><!--
				function sq_set_check_all(form, prefix, value) {
					set_els = form.elements[prefix];

					if (set_els.length) {
						for (k = 0; k < set_els.length; k++) {
							set_els[k].checked = value;
						}
					} else {
						set_els.checked = value;
					}
				}

				function sq_test_check_all(form, prefix, set_el) {
					test_els = form.elements[prefix];
					set_el = form.elements[set_el];

					if (test_els.length) {
						test = true;
						for (k = 0; k < test_els.length; k++) {
							if (!test_els[k].checked) {
								test = false;
								break;
							}
						}
					} else {
						test = test_els.checked;
					}

					set_el.checked = test;
				}
			//--></script>
			<table class="sq-backend-table">
			<tr>
			<td class="sq-form-sub-header" colspan="2"><b><?php
				hidden_field($prefix.'_submission_offset', $submission_offset);
				if ($submission_offset > 0) {
					?><a title="<?php echo translate('Go to the first page of submissions') ?>" style="text-decoration: none; color: #fff" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_submission_offset', '0'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php

				} else {
					?><span title="<?php echo translate('This is the first page; you cannot go any further back') ?>" style="color: #333"><?php

				}
				?>
				&lt;&lt;
				<?php
				if ($submission_offset > 0) {
					?></a><?php
				} else {
					?></span><?php
				}
				?> &nbsp; &nbsp; <?php
				if ($submission_offset > 0) {
					?><a title="<?php echo translate('Go to the previous page of submissions') ?>" style="text-decoration: none; color: #fff" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_submission_offset', '<?php echo $submission_offset - $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php

				} else {
					?><span title="<?php echo translate('This is the first page; you cannot go any further back') ?>" style="color: #333"><?php

				}
				?>
				&lt;
				<?php
				if ($submission_offset > 0) {
					?></a><?php
				} else {
					?></span><?php
				}
				?>
				&nbsp; &nbsp; <?php echo sprintf(translate('Page %1$s of %2$s'), round(($submission_offset + $num_per_page) / $num_per_page), ceil($asset_count / $num_per_page)); ?> &nbsp; &nbsp;

				<?php
				if (($submission_offset + $num_per_page) < $asset_count) {
					?><a title="<?php echo translate('Go to the next page of submissions') ?>" style="text-decoration: none; color: #fff" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_submission_offset', '<?php echo $submission_offset + $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php

				} else {
					?><span title="<?php echo translate('This is the last page; you cannot go any further forward') ?>" style="color: #333"><?php

				}
				?>
				&gt;
				<?php
				if (($submission_offset + $num_per_page) < $asset_count) {
					?></a><?php
				} else {
					?></span><?php
				}
				?> &nbsp; &nbsp; <?php
				if (($submission_offset + $num_per_page) < $asset_count) {
					?><a title="<?php echo translate('Go to the last page of submissions') ?>" style="text-decoration: none; color: #fff" href="#" onClick="Javascript: set_hidden_field('<?php echo $prefix ?>_submission_offset', '<?php echo ($num_pages - 1) * $num_per_page; ?>'); set_hidden_field('process_form', '0'); submit_form(); return false;"><?php

				} else {
					?><span title="<?php echo translate('This is the last page; you cannot go any further forward') ?>" style="color: #333"><?php

				}
				?>
				&gt;&gt;
				<?php
				if (($submission_offset + $num_per_page) < $asset_count) {
					?></a><?php
				} else {
					?></span><?php
				}
				?></b>
			</td>
			<td class="sq-form-sub-header" colspan="5" align="right"><?php echo sprintf(translate('Displaying submissions <b>%1$s</b> to <b>%2$s</b>, out of <b>%3$s</b    > total submissions'), $submission_offset + 1, min($asset_count, $submission_offset + $num_per_page), $asset_count) ?>

			</td>
			</tr>
			<tr>
				<th>
					<?php echo translate('Asset ID') ?>

				</th>
				<th>
					<?php echo translate('Submission Time') ?>

				</th>
				<th>
					<?php echo translate('User') ?>

				</th>
				<th>
					<?php echo translate('Score') ?>

				</th>
				<th>
					<input type="checkbox" name="<?php echo $prefix ?>_expand_all" value="" onclick="sq_set_check_all(this.form, '<?php echo $prefix ?>_expand[]', this.checked);"> <?php echo translate('Expand?') ?>

				</th>
			<?php
			if ($write_access) {
				?>
				<th>
					<input type="checkbox" name="<?php echo $prefix ?>_delete_all" value="" onclick="sq_set_check_all(this.form, '<?php echo $prefix ?>_delete[]', this.checked);"> <?php echo translate('Delete?') ?>

				</th>
				<?php
			}
			?>
				<th>&nbsp;</th>
			</tr>
			<?php
			// For memory reasons, we are going to fetch this one row at a time
			// because we could be displaying a lot on this page
			$result = NULL;
			 try {
				 $_query = MatrixDAL::preparePdoQuery($sql);
				 MatrixDAL::bindValueToPdo($_query, 'assetid',           $asset->id);
				 MatrixDAL::bindValueToPdo($_query, 'subfolder_assetid', $sub_folder->id);
				 MatrixDAL::bindValueToPdo($_query, 'inhd_type_code',    'online_quiz_submission');
				 MatrixDAL::bindValueToPdo($_query, 'from_date',         $from_value);
				 MatrixDAL::bindValueToPdo($_query, 'to_date',           $to_value);
				 $result = MatrixDAL::executePdoAssoc($_query);
			} catch (Exception $e) {
				throw new Exception($e->getMessage());
			}

			foreach ($result as $k => $asset_info) {
				$assetid = $asset_info['assetid'];

				// time is good, load me up
				$sub_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid);
				$sub_user = $asset_info['created_userid'];
				$sub_time = date('d M Y H:i:s', strtotime($asset_info['created']));
				$sub_expanded = array_search($assetid, $expand_list) !== FALSE;
				$num_subs++;

				if ($num_subs > $num_per_page) break;
				?>
				<tr<?php
					echo ($sub_expanded) ? ' class="alt"' : '';
				?>>
					<td>
						<?php echo $assetid ?>
					</td>
					<td>
						<?php echo $sub_time ?>
					</td>
					<td>
						<?php
						$valid_user = FALSE;
						if (!empty($sub_user)) {
							$sub_user_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($sub_user);
							if (!empty($sub_user_asset)) {
								echo get_asset_tag_line($sub_user);
								$valid_user = TRUE;
							}
						}
						if (!$valid_user) {
							?><i><?php echo translate('No logged in user'); ?></i><?php

						}
						?>
					</td>
					<td> <?php echo $sub_asset->getScore(); ?></td>
					<td>
						<input type="checkbox" name="<?php echo $prefix ?>_expand[]" value="<?php echo $assetid ?>" <?php if ($sub_expanded) echo ' checked="checked"'; ?> onclick="sq_test_check_all(this.form, '<?php echo $prefix ?>_expand[]', '<?php echo $prefix ?>_expand_all');">
					</td>
				<?php
				if ($write_access) {
					?>
					<td>
						<input type="checkbox" name="<?php echo $prefix ?>_delete[]" value="<?php echo $assetid ?>" onclick="sq_test_check_all(this.form, '<?php echo $prefix ?>_delete[]', '<?php echo $prefix ?>_delete_all');">
					</td>
					<?php
				}
				?>
					<td>[ <a href="<?php echo $sub_asset->getBackendHref(); ?>"><?php echo translate('View') ?></a> ]</td>

				</tr>
				<?php
				// display only if expanded
				if ($sub_expanded) {
					?>
					<tr>
						<td colspan="6" class="sq-form-sub">
							<table class="sq-backend-table">
								<tr>
									<?php
									$i = 0;
									$incl_user_details = in_array('__user_details__',$expand_sections) ? TRUE : FALSE;
									$incl_quiz_responses = in_array('__quiz_responses__', $expand_sections) ? TRUE : FALSE;

									$elements = $sub_asset->getSummary($incl_user_details, $incl_quiz_responses);
									if (count($elements) % 2 != 0) {
										$elements[] = Array('name' => '', 'answer' => '');
									}
									foreach ($elements as $element) {
										$i++;
										?>
										<th><?php echo $element['name'] ?>:</th>
										<td><?php echo htmlspecialchars($element['answer']); ?></td>
										<?php
										if (($i < count($elements)) && ($i % 2 == 0)) {
											?>
								</tr>
								<tr>
											<?php
										}
									}//end foreach elemnent
									?>
								</tr>
							</table>
						</td>
					</tr>
					<?php
				}//end if expanded
			}//end while submission
			?>
			</table>
			<script type="text/javascript"><!--
			sq_test_check_all(document.main_form, '<?php echo $prefix ?>_expand[]', '<?php echo $prefix ?>_expand_all');
			//--></script>
		<?php
		}//end else


		return TRUE;

	}//end paintSubmissionList()


	/**
	* Processes the submissions block (only processes deletion of submissions)
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processSubmissionList(&$asset, &$o, $prefix)
	{
		$delete_list = isset($_POST[$prefix.'_delete']) ? $_POST[$prefix.'_delete'] : Array();
		if (empty($delete_list)) return FALSE;

		$GLOBALS['SQ_SYSTEM']->changeDatabaseConnection('db2');
		$GLOBALS['SQ_SYSTEM']->doTransaction('BEGIN');

		// acquire the links lock on the form
		if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($asset->id, 'links')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		foreach ($delete_list as $delete_assetid) {
			// is the assetid a valid one?
			if (!assert_valid_assetid($delete_assetid, '', FALSE, FALSE)) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}

			// acquire the links lock on the submission to delete
			if (!$GLOBALS['SQ_SYSTEM']->am->acquireLock($delete_assetid, 'links')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
			$submissions_folder = $asset->getSubmissionsFolder();
			$submission_link = $GLOBALS['SQ_SYSTEM']->am->getLinkByAsset($submissions_folder->id, $delete_assetid, SQ_LINK_TYPE_3);
			if (!$submission_link) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			} else {
				if (!$submissions_folder->deleteLink($submission_link['linkid'])) {
					$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
					$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
					return FALSE;
				}
			}

			// release the links lock
			if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($delete_assetid, 'links')) {
				$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
				$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
				return FALSE;
			}
		}//end foreach
		// now release the links lock on the form itself
		if (!$GLOBALS['SQ_SYSTEM']->am->releaseLock($asset->id, 'links')) {
			$GLOBALS['SQ_SYSTEM']->doTransaction('ROLLBACK');
			$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();
			return FALSE;
		}

		$GLOBALS['SQ_SYSTEM']->doTransaction('COMMIT');
		$GLOBALS['SQ_SYSTEM']->restoreDatabaseConnection();

		return TRUE;

	}//end processSubmissionList()


	/**
	* Processes the Download Button press
	*
	* This function will pass off the work to processExportCSV() or processExportXML(), as appropriate.
	* If we aren't downloading a file, this function will silently return a success.
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processDownloadButton(&$asset, &$o, $prefix)
	{
		$export_button_pressed = array_get_index($_POST, $prefix.'_export_switch');
		if (is_null($export_button_pressed)) return TRUE;

		$export_type = array_get_index($_POST, $prefix.'_export_format', '');
		$export_delimiter = array_get_index($_POST, $prefix.'_delimiter', '');

		// Grab the asset IDs of the export fields as the value of the array. Must select some fields for export to proceed
		if (isset($_POST[$prefix.'_export_fields'])) {
			$export_fields = $_POST[$prefix.'_export_fields'];

			if (!empty($export_fields)) {
				switch ($export_type) {
					case 'csv':
						$this->processExportCSV($asset, $o, $prefix, $export_fields, TRUE, $export_delimiter);
					break;

					case 'csv_nhr':
						$this->processExportCSV($asset, $o, $prefix, $export_fields, FALSE, $export_delimiter);
					break;
				}
			}
		}

		return TRUE;

	}//end processDownloadButton()


	/**
	* Exports chosen submissions to a comma separated value (CSV) file
	*
	* Will only perform export for the date range and fields selected in the Export block.
	*
	* @param object		&$asset			the asset whose interface we are painting
	* @param object		&$o				the outputter class
	* @param string		$prefix			prefix for the form elements
	* @param array		$export_fields	set of question IDs to export
	* @param boolean	$header_row		whether the header row is filled in
	* @param string		$delimiter		delimiter char
	*
	* @return boolean
	* @access public
	*/
	function processExportCSV(&$asset, &$o, $prefix, $export_fields=Array(), $header_row=TRUE, $delimiter=',')
	{
		require_once SQ_FUDGE_PATH.'/csv/csv.inc';
		$csv = new CSV();
		$csv->setFilename('submission_log.csv');
		$csv->setDeliminator($delimiter);

		// get the from and to values
		$from_value = isset($_POST[$prefix.'_export_fromvalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_export_fromvalue']['m'],$_POST[$prefix.'_export_fromvalue']['d'],$_POST[$prefix.'_export_fromvalue']['y'])) : '---------- --:--:--';
		$to_value = isset($_POST[$prefix.'_export_tovalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_export_tovalue']['m'],$_POST[$prefix.'_export_tovalue']['d'],$_POST[$prefix.'_export_tovalue']['y'])) : '---------- --:--:--';

		$from_value = explode('-',iso8601_date_component($from_value));
		$to_value = explode('-',iso8601_date_component($to_value));

		// convert the ISO times to time stamps but manually because we can't be sure
		// about the h:m:s section of the above times
		$from_ts = mktime(0,0,0,$from_value[1],$from_value[2],$from_value[0]);
		$to_ts   = mktime(23,59,59,$to_value[1],$to_value[2],$to_value[0]);

		$logs = Array();
		$headers = Array();

		$export_time_field = in_array('__time__', $export_fields);
		$export_user_field = in_array('__user__', $export_fields);
		$export_ipaddress_field = in_array('__ipaddress__', $export_fields);
		$export_score_field = in_array('__score__', $export_fields);

		if ($export_time_field) {
			$headers['__time__'] = translate('Submission Time');

		}
		if ($export_user_field) {
			$headers['__user__'] = translate('User ID');

		}
		if ($export_ipaddress_field) {
			$headers['__ipaddress__'] = translate('IP Address');

		}
		if ($export_score_field) {
			$headers['__score__'] = translate('Score');

		}

		$sub_folder = $asset->getSubmissionsFolder();

		$base_sql = 'SELECT
				a.assetid, a.type_code, a.created, a.created_userid
				FROM
				('.SQ_TABLE_RUNNING_PREFIX.'ast a JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_typ_inhd i ON a.type_code = i.type_code)
				JOIN '.SQ_TABLE_RUNNING_PREFIX.'ast_lnk l
				ON l.minorid = a.assetid';

		$where = 'l.majorid IN ('.MatrixDAL::quote($asset->id).', '.MatrixDAL::quote($sub_folder->id).') AND i.inhd_type_code = '.MatrixDAL::quote('online_quiz_submission').' AND a.created BETWEEN '.MatrixDAL::quote(ts_iso8601($from_ts)).' AND '.MatrixDAL::quote(ts_iso8601($to_ts));

		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'a');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'i');
		$where = $GLOBALS['SQ_SYSTEM']->constructRollbackWhereClause($where, 'l');

		// This could be a large download, however DAL does not easily support
		// row by row fetching, so we will do it in chunks of 1000, or however
		// many we get back. If we don't get that many, we will continue to
		// query until there are no more, to pick up any additional
		// submissions made since the first query was made.
		$offset = 0;
		$chunk_size = 1000;

		while (TRUE) {
			// Get the current limited query
			$sql = db_extras_modify_limit_clause($base_sql.$where, MatrixDAL::getDbType(), $chunk_size, $offset);
			$result = MatrixDAL::executeSqlAssoc($sql);

			// If no further results, we are done.
			if (count($result) == 0) break;

			foreach ($result as $asset_info) {
				$assetid = $asset_info['assetid'];
				$type_code = $asset_info['type_code'];

				$create_time = strtotime($asset_info['created']);

				$sub_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $type_code);
				if (!$sub_asset->attr('complete')) continue;

				// get the submission ip
				$ipaddress = $sub_asset->getIP();
				$score = $sub_asset->getScore();

				// get the summary
				$logs[$assetid] = Array();

				if ($export_time_field) {
					$logs[$assetid]['__time__'] = date('Y-m-d H:i:s', $create_time);
				}
				if ($export_user_field) {
					$logs[$assetid]['__user__'] = $asset_info['created_userid'];
				}
				if ($export_ipaddress_field) {
					$logs[$assetid]['__ipaddress__'] = $ipaddress;
				}
				if ($export_score_field) {
					$logs[$assetid]['__score__'] = $score;
				}

				$submission_summary = $sub_asset->getSummary();

				// Export full Question Text and Question Answer Text
				foreach ($submission_summary as $question_id => $details) {
					// Now we're only interested in fields that have been specified for export
					if (in_array($question_id, $export_fields)) {
						if (!isset($headers[$question_id])) {
							$headers[$question_id] = $details['name'];
						}
						$logs[$assetid][$question_id] = $details['answer'];
					}
				}

			}//end foreach

			// Advance the chains along, and prepare to get more records
			$offset =+ count($result);

		}//end while - there are more records

		if ($header_row) $csv->setFieldHeaders($headers);

		$csv->setValues($logs);

		// export with keyed fields
		$csv->export(TRUE);
		exit(0);

		return TRUE;

	}//end processExportCSV()


	/**
	* Paint Question Pool Setup
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintQuestionPoolSetup(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');
		// get existing root links (pools)
		$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'root');
		$pool_setup = $asset->attr('question_pool_setup');

		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header" >
					<?php echo translate('Pool Node'); ?>

				</td>
				<td class="sq-backend-table-header">
					<?php echo translate('Pool Size'); ?>

				</td>
				<td class="sq-backend-table-header">
					<?php echo translate('Retries per Question'); ?>

				</td>
				<td class="sq-backend-table-header">
					<?php echo translate('Pool Order'); ?>

				</td>
			</tr>
		<?php
		$order = 1;
		foreach ($root_links as $link){
			?>
			<tr>
				<td class="sq-backend-table-cell">
				<?php
					echo get_asset_tag_line($link['minorid'], 'details');
				?>
				</td>
				<td class="sq-backend-table-cell">
				<?php
				$size = (!empty($pool_setup[$link['minorid']]['size'])) ? $pool_setup[$link['minorid']]['size'] : 0;
				if ($write_access){
					text_box($prefix.'_size_'.$link['minorid'], $size);
				} else {
					echo $size;
				}
				?>
				</td>
				<td class="sq-backend-table-cell">
				<?php
				$retries = (!empty($pool_setup[$link['minorid']]['retries'])) ? $pool_setup[$link['minorid']]['retries'] : 0;
				if ($write_access){
					text_box($prefix.'_retries_'.$link['minorid'], $retries);
				} else {
					echo $retries;
				}
				?>
				</td>
				<td class="sq-backend-table-cell">
				<?php
				if ($write_access){
					text_box($prefix.'_order_'.$link['minorid'], $order);
				} else {
					echo $order;
				}
				?>
				</td>
			</tr>
		<?php
			$order++;
		}
		?>
		</table>
		<?php
		hidden_field($prefix.'_interactive_mode', '1');

		return TRUE;

	}//end paintQuestionPoolSetup()


	/**
	* Process Question Pool Setup
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processQuestionPoolSetup(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;
		if (!isset($_POST[$prefix.'_interactive_mode'])) return TRUE;

		//adjust the order first
		$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'root');
		foreach ($root_links as $link){
			$order = (isset($_POST[$prefix.'_order_'.$link['minorid']])) ? (int)$_POST[$prefix.'_order_'.$link['minorid']] : $link['sort_order'];
			$GLOBALS['SQ_SYSTEM']->am->updateLink($link['linkid'], $link['link_type'], $link['value'], $order, $link['locked']);
		}

		$pool_setup = Array();
		$root_links = $GLOBALS['SQ_SYSTEM']->am->getLinks($asset->id, SQ_LINK_NOTICE, '', FALSE, 'major', 'root');
		foreach ($root_links as $link){
			$retries = (isset($_POST[$prefix.'_retries_'.$link['minorid']])) ? (int)$_POST[$prefix.'_retries_'.$link['minorid']] : 0;
			$size = (isset($_POST[$prefix.'_size_'.$link['minorid']])) ? (int)$_POST[$prefix.'_size_'.$link['minorid']] : 0;
			$children = $GLOBALS['SQ_SYSTEM']->am->getChildren($link['minorid'], 'online_quiz_question_multichoice', TRUE);
			if (($size + ($size*$retries)) > (count($children))){
				trigger_localised_error('CMS0113', sprintf(translate('Not enough questions in the pool for pool root node (Id: #%s).'), $link['minorid']), E_USER_WARNING);
			}

			$pool_setup[$link['minorid']] = Array (
												'retries' => $retries,
												'size'	  => $size,
											);
		}

		$asset->setAttrValue('question_pool_setup', $pool_setup);
		$asset->saveAttributes();

		return TRUE;

	}//end processQuestionPoolSetup()


	/**
	* Paints the submission asset sections available for expanding in the submission list
	*
	* @param object	&$asset	the asset whose interface we are painting
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintExpandSections(&$asset, &$o, $prefix)
	{

		$expand_sections = isset($_POST[$prefix.'_expand_sections']) ? $_POST[$prefix.'_expand_sections'] : Array();

		//default is both checked and if you uncheck both, they'll be rechecked
		$checked = ($expand_sections) ? in_array('__user_details__', $expand_sections) : TRUE;
		echo check_box($prefix.'_expand_sections[]', '__user_details__', $checked).'User Details Form Questions<br/>';
		$checked = ($expand_sections) ? in_array('__quiz_responses__', $expand_sections) : TRUE;
		echo check_box($prefix.'_expand_sections[]', '__quiz_responses__', $checked).'Quiz Questions<br/>';

	}//end paintExpandSections()


	/**
	* Paint the question option format interface
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintQuestionOptionFormat(&$asset, &$o, $prefix)
	{
		$write_access = $asset->writeAccess('attributes');

		if ($write_access) {
			text_area($prefix.'_question_option_format', $asset->attr('question_option_format'), 0, 0, 0, 'style="width: 300px; height: 100px;"');
		} else {
			echo htmlentities($asset->attr('question_option_format'));
		}

		return TRUE;

	}//end paintQuestionOptionFormat()


	/**
	* Process the question option format interface
	*
	* @param object	&$asset	the asset to which we belong
	* @param object	&$o		the outputter class
	* @param string	$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function processQuestionOptionFormat(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return FALSE;

		if (isset($_REQUEST[$prefix.'_question_option_format']) === TRUE) {
			$question_option_format = array_get_index($_REQUEST, $prefix.'_question_option_format', '');
			$asset->setAttrValue('question_option_format', trim($question_option_format));
		}

		return TRUE;

	}//end processQuestionOptionFormat()


	/**
	* Show-if conditional: isListingMode
	*
	* @param object &$asset asset being painted
	*
	* @return boolean
	* @access public
	*/
	function isListingMode(&$asset)
	{
		return ($asset->attr('interactive_mode')) ? FALSE : TRUE;

	}//end isListingMode()


	/**
	* Show-if conditional: isInteractiveMode
	*
	* @param object &$asset asset being painted
	*
	* @return boolean
	* @access public
	*/
	function isInteractiveMode(&$asset)
	{
		return ($asset->attr('interactive_mode')) ? TRUE : FALSE;

	}//end isInteractiveMode()


}//end class

?>
