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
* $Id: form_email_edit_fns.inc,v 1.22 2004/07/07 00:34:47 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once dirname(__FILE__).'/../form/form_edit_fns.inc';
require_once SQ_FUDGE_PATH.'/datetime_field/datetime_field.inc';

/**
* Form_Email_Edit_Fns
*
* Purpose
*
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Form_Email_Edit_Fns extends Form_Edit_Fns
{
	

	/**
	* Constructor
	*
	*/
	function Form_Email_Edit_Fns()
	{
		$this->Form_Edit_Fns();

	}//end constructor


//--        SUBMISSIONS        --//


	/**
	* Paints the submissions block
	*
	* @param object Form_Email			&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintSubmissions(&$asset, &$o, $prefix) 
	{
		if (!$asset->writeAccess('attributes')) return false;

		$expand_list = isset($_POST[$prefix.'_viewsub']) ? array_keys($_POST[$prefix.'_viewsub']) : Array();
		$num_subs = 0;

		$parameters = Array(
			'min'			=> '2003-01-01 00:00:00',
			'max'			=> ts_iso8601(time()),
			'allow_circa'	=> '0',
			'show'			=> Array('y', 'm', 'd'),
			'null'			=> Array(),
			'style'			=> Array(
						'y' => 's',
						'm' => 's',
						'd' => 's',
					),
			);

		$parameters['print_format'] = 'j M Y';

		$from_value = isset($_POST[$prefix.'_fromvalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_fromvalue']['m'],$_POST[$prefix.'_fromvalue']['d'],$_POST[$prefix.'_fromvalue']['y'])) : '---------- --:--:--';
		$to_value = isset($_POST[$prefix.'_tovalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_tovalue']['m'],$_POST[$prefix.'_tovalue']['d'],$_POST[$prefix.'_tovalue']['y'])) : '---------- --:--:--';

		$from = new Datetime_Field($prefix.'_from', $from_value, $parameters);
		$to = new Datetime_Field($prefix.'_to', $to_value, $parameters);

		echo 'View submissions from: ';
		$from->printField();
		echo ' to ';
		$to->printField();

		$o->closeField();

		$o->openField('Submissions');

		$from_value = explode('-',substr($from_value,0,10));
		$to_value = explode('-',substr($to_value,0,10));

		// convert the ISO times to time stamps but manually because we can't be sure
		// about the h:m:s section of the above times
		$from_ts = mktime(0,0,0,$from_value[1],$from_value[2],$from_value[0]);
		$to_ts   = mktime(23,59,59,$to_value[1],$to_value[2],$to_value[0]);

		$log_path = $asset->data_path.'/submission_logs';
		if (!file_exists($log_path.'/')) return;
		?>
		<table class="sq-backend-table">
		<tr>
			<td class="sq-backend-table-header">
				ID
			</td>
			<td colspan="2" class="sq-backend-table-header">
				Submission Time
			</td>
			<td colspan="2" class="sq-backend-table-header">
				User
			</td>
			<td class="sq-backend-table-header">
				Expand ?
			</td>
		</tr>
		<?php

		// search in the sub-logs directory for files that fit in the date range
		if ($handle = opendir($log_path)) {
			while (false !== ($file = readdir($handle))) {
				if ($file == '.' || $file == '..') continue;			
				$log_file = $log_path.'/'.$file;
				$sub_id = (int)substr($file,4,strpos($file,'.')-4);

				$file_ts = filemtime($log_file);

				if (($file_ts >= $from_ts) && ($file_ts <= $to_ts)) {
					// time is good; load me up
					$tree = new XML_Tree($log_file);
					$root = &$tree->getTreeFromFile();
					if (PEAR::isError($root)) {
						pre_echo($root->getMessage()."\n".$root->getUserInfo());
						return;
					}

					$num_subs++;

					$sub_time = date('j M Y H:i:s', iso8601_ts($root->getAttribute('time')));
					$sub_user = $root->getAttribute('user');
					$sub_expanded = array_search($sub_id, $expand_list) !== false;
					?>
						<tr>
						<td class="sq-backend-table-cell-alt">
							<?php echo $sub_id ?>
						</td>
						<td class="sq-backend-table-cell-alt" colspan="2">
							<?php echo $sub_time ?>
						</td>
						<td class="sq-backend-table-cell-alt" colspan="2">
							<?php if (!is_null($sub_user)) {									
								$sub_user_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($sub_user);
								echo $sub_user_asset->name . ' (Id: #'.$sub_user.')';
							} ?>
						</td>
						<td class="sq-backend-table-cell-alt">
							<input type="checkbox" name="<?php echo $prefix ?>_viewsub[<?php echo $sub_id ?>]" value="1" <?php if ($sub_expanded) echo ' checked="checked"' ?>>
						</td>
					</tr>
					<?php
						// display only if expanded
						if ($sub_expanded) { ?>
						<tr>
						<?php
	
						$elements = &$this->_generateSubmissionList($root);

						$i = 0;
						foreach($elements as $element) {
							$i++;
	
							if ($i % 2 == 1) { ?>
								<td class="sq-backend-table-cell">&nbsp;</td>
							<?php }
							?>
							<td class="sq-backend-table-cell">
								<i><?php echo $element['name'] ?>:</i>
							</td>
							<td class="sq-backend-table-cell">
								<?php echo $element['content'] ?>
							</td>
							<?php 
							if (($i < count($elements)) && ($i % 2 == 0)) { ?>
								</tr><tr>
						<?php }
						}	// end-if sub_expanded
		
						?></tr><?php
		
						$kids =& $root->children;
		
						for (reset($kids); null !== ($k = key($kids)); next($kids)) {
							$this_child =& $kids[$k];
							if ($this_child->name == 'section') {
							?>
							<tr>
								<td class="sq-backend-table-cell">&nbsp;</td>
								<td class="sq-backend-table-cell-alt" colspan="5">
									<b>Section: <?php echo $this_child->getAttribute('name') ?></b>
								</td>
							</tr>
							<tr>
							<?php
								$elements = &$this->_generateSubmissionList($this_child);
	
								$i = 0;
								foreach ($elements as $element) {
									$i++;
									if (($i % 2 == 1)) {
										?><td class="sq-backend-table-cell">&nbsp;</td><?php
									}
									?>
									<td class="sq-backend-table-cell">
										<i><?php echo $element['name'] ?>:</i>
									</td>
									<td class="sq-backend-table-cell">
										<?php echo $element['content'] ?>
										</td>
									<?php 
									if (($i < count($elements)) && ($i % 2 == 0)) {
										?></tr><tr><?php
									}
								}
								?></tr><?php
								
							}//end if this element is a section

						}//end foreach root element

					}//end if this element expanded

				}//end iff file's datestamp in range

			}//end while reading directories

		}//end handle opened
		closedir($handle);
		?>
		</table>
		<?php

	}//end paintSubmissions()


	/**
	* Processes the submissions block (nothing to process!)
	*
	* @param object Form_Email			&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processSubmissions(&$asset, &$o, $prefix)
	{
		return false;

	}//end processSubmissions()


	/**
	* Paints the export block
	*
	* @param object Form_Email			&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintExport(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return false;

		$parameters = Array(
			'min'			=> '2003-01-01 00:00:00',
			'max'			=> ts_iso8601(time()),
			'allow_circa'	=> '0',
			'show'			=> Array('y', 'm', 'd'),
			'null'			=> Array(),
			'style'			=> Array(
						'y' => 's',
						'm' => 's',
						'd' => 's',
					),
			);

		$parameters['print_format'] = 'j M Y';

		$from_value = isset($_POST[$prefix.'_export_fromvalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_export_fromvalue']['m'],$_POST[$prefix.'_export_fromvalue']['d'],$_POST[$prefix.'_export_fromvalue']['y'])) : '---------- --:--:--';
		$to_value = isset($_POST[$prefix.'_export_tovalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_export_tovalue']['m'],$_POST[$prefix.'_export_tovalue']['d'],$_POST[$prefix.'_export_tovalue']['y'])) : '---------- --:--:--';

		$from = new Datetime_Field($prefix.'_export_from', $from_value, $parameters);
		$to = new Datetime_Field($prefix.'_export_to', $to_value, $parameters);

		$from->printField();
		echo ' to ';
		$to->printField();

		$o->closeField();

		$o->openField('Format');

		$options = Array('' => '------- Please Select -------',
						 'csv' => 'Comma Separated Values (CSV)',
						 'xml' => 'XML Format');

		echo combo_box($prefix.'_export_format', $options, false, '');
		echo hidden_field($prefix.'_export_switch', '0');
		
		$o->closeField();

		// explanation for onClick for the Download button, there is some trickery involved here >:)
		// - set export switch to 1 = "ON"
		// - submit form (automatically sets SQ_FORM_SUBMITTED) - we're still here though, as the 
		//   browser will be sent through the CSV/XML file and it will be offered to save - not display
		//   (DEPENDS on browser - CSV should be offered to save, XML could be intercepted by browser!)
		// - unset SQ_FORM_SUBMITTED so the form can still be submitted via Commit
		// - set export switch to 0 = "OFF" so Commit doesn't try to save a file either
		// - return true for the event as per JS standard

		$o->openField('Download');

		echo normal_button($prefix.'_export', 'Download File', 'this.form.'.$prefix.'_export_switch.value = 1; this.form.submit(); SQ_FORM_SUBMITTED = false; this.form.'.$prefix.'_export_switch.value = 0; return true;');

		return true;

	}//end paintExport()


	/**
	* Processes the export block
	*
	* This function will pass off the work to processExportCSV() or processExportXML(), as appropriate.
	* If we aren't downloading a file, this function will silently return a success.
	*
	* @param object Form_Email			&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processExport(&$asset, &$o, $prefix)
	{
		if (!$_POST[$prefix.'_export_switch']) return true;
		$export_type = isset($_POST[$prefix.'_export_format']) ? $_POST[$prefix.'_export_format'] : '';
		switch($export_type) {
			case 'csv':
				$this->processExportCSV($asset, $o, $prefix);
			break;

			case 'xml':
				$this->processExportXML($asset, $o, $prefix);
			break;
		}

		return true;

	}//end processExport()


	/**
	* Exports chosen submissions to a comma separated value (CSV) file
	*
	* Will only perform export for the date range which was
	* selected in the Export block.
	*
	* @param object Form_Email			&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processExportCSV(&$asset, &$o, $prefix)
	{
		require_once SQ_FUDGE_PATH.'/csv/csv.inc';
		$csv = new CSV();
		$csv->setFilename('submission_log.csv');

		// get the from and to values
		$from_value = isset($_POST[$prefix.'_export_fromvalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_export_fromvalue']['m'],$_POST[$prefix.'_export_fromvalue']['d'],$_POST[$prefix.'_export_fromvalue']['y'])) : '---------- --:--:--';
		$to_value = isset($_POST[$prefix.'_export_tovalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_export_tovalue']['m'],$_POST[$prefix.'_export_tovalue']['d'],$_POST[$prefix.'_export_tovalue']['y'])) : '---------- --:--:--';

		$from_value = explode('-',substr($from_value,0,10));
		$to_value = explode('-',substr($to_value,0,10));

		// convert the ISO times to time stamps but manually because we can't be sure
		// about the h:m:s section of the above times
		$from_ts = mktime(0,0,0,$from_value[1],$from_value[2],$from_value[0]);
		$to_ts   = mktime(23,59,59,$to_value[1],$to_value[2],$to_value[0]);

		$log_path = $asset->data_path.'/submission_logs';
		$logs = Array();
		$headers = Array('__time__' => 'Submission Time', '__user__' => 'User Submitting Form');

		// search submission logs directory for files in the date range
		if ($handle = opendir($log_path)) {
			while (false !== ($file = readdir($handle))) {
				if ($file == '.' || $file == '..') continue;			
				$log_file = $log_path.'/'.$file;
				$sub_id = (int)substr($file,4,strpos($file,'.')-4);

				$file_ts = filemtime($log_file);

				if (($file_ts >= $from_ts) && ($file_ts <= $to_ts)) {
					// time is good; load me up
					$tree = new XML_Tree($log_file);
					$root  = &$tree->getTreeFromFile();
					$logs[$sub_id] = Array();
					if (PEAR::isError($root)) {
						pre_echo($root->getMessage()."\n".$root->getUserInfo());
						return;
					}

					$sub_time = str_replace('T',' ',$root->getAttribute('time'));
					$sub_user = $root->getAttribute('user');
					$logs[$sub_id]['__time__'] = $sub_time;
					if (!is_null($sub_user)) {									
						$sub_user_asset = $GLOBALS['SQ_SYSTEM']->am->getAsset($sub_user);
						$logs[$sub_id]['__user__'] = $sub_user_asset->name . ' (Id: #'.$sub_user.')';
					} else {
						$logs[$sub_id]['__user__'] = '';
					}

					$elements = &$this->_generateSubmissionList($root);
	
					foreach($elements as $element) {
						$logs[$sub_id][$element['id']] = $element['content'];
						if (!isset($headers[$element['id']])) {
							$headers[$element['id']] = $element['name'];
						}
					}

					$kids =& $root->children;
		
					for (reset($kids); null !== ($k = key($kids)); next($kids)) {
						$this_child =& $kids[$k];
						if ($this_child->name == 'section') {
							$elements = &$this->_generateSubmissionList($this_child);
							foreach($elements as $element) {
								$logs[$sub_id][$element['id']] = $element['content'];
								if (!isset($headers[$element['id']])) {
									$headers[$element['id']] = $element['name'];
								}
							}
						}
					}

				}//end if file's datestamp in range

			}//end while reading directories

		}//end if handle opened
		closedir($handle);

		$csv->setFieldHeaders($headers);
		$csv->setValues($logs);
		$csv->export();
		exit(0);

		return true;

	}//end processExportCSV()


	/**
	* Exports chosen submissions to an XML format
	*
	* XML format is the same format used to store the submission logs, with an overarching
	* "submissions" root tag around it. Will only perform export for the date range which was
	* selected in the Export block.
	*
	* @param object Form_Email			&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processExportXML(&$asset, &$o, $prefix)
	{
		header('Content-Type: application/xml');

		// defining the file as attachment forces most browsers to offer it for download
		header('Content-Disposition: attachment; filename=submission_log.xml;');

		// get the from and to values
		$from_value = isset($_POST[$prefix.'_export_fromvalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_export_fromvalue']['m'],$_POST[$prefix.'_export_fromvalue']['d'],$_POST[$prefix.'_export_fromvalue']['y'])) : '---------- --:--:--';
		$to_value = isset($_POST[$prefix.'_export_tovalue']) ? ts_iso8601(mktime(0,0,0,$_POST[$prefix.'_export_tovalue']['m'],$_POST[$prefix.'_export_tovalue']['d'],$_POST[$prefix.'_export_tovalue']['y'])) : '---------- --:--:--';

		$from_value = explode('-',substr($from_value,0,10));
		$to_value = explode('-',substr($to_value,0,10));

		// convert the ISO times to time stamps but manually because we can't be sure
		// about the h:m:s section of the above times
		$from_ts = mktime(0,0,0,$from_value[1],$from_value[2],$from_value[0]);
		$to_ts   = mktime(23,59,59,$to_value[1],$to_value[2],$to_value[0]);

		$log_path = $asset->data_path.'/submission_logs';
		$logs = Array();
		$headers = Array('__time__' => 'Submission Time', '__user__' => 'User Submitting Form');

		echo '<submissions>';

		// search submission logs directory for files in the date range
		if ($handle = opendir($log_path)) {
			while (false !== ($file = readdir($handle))) {
				if ($file == '.' || $file == '..') continue;			
				$log_file = $log_path.'/'.$file;
				$sub_id = (int)substr($file,4,strpos($file,'.')-4);

				$file_ts = filemtime($log_file);
				if (($file_ts >= $from_ts) && ($file_ts <= $to_ts)) {
					// time is good; load me up and print
					echo file_to_string($log_file);
				}
			}
		}
		closedir($handle);
		
		echo '</submissions>';
		exit(0);

		return true;

	}//end processExportXML()


	/**
	* Returns a list of name-value pairs for a submission result
	*
	* @param object XML_Tree_Node &$tree_root the XML node that we are going to process
	*
	* @return array with each element containing indices ('name', 'id', 'content')
	* @access private
	*/
	function &_generateSubmissionList(&$tree_root)
	{
		$children = Array();

		$kids =& $tree_root->children;

		for (reset($kids); null !== ($k = key($kids)); next($kids)) {
			$this_child =& $kids[$k];
			$child_id = $this_child->getAttribute('id');
			$child_name = $this_child->getAttribute('name');
			switch($this_child->name) {
				case 'section':
				break;

				case 'text_q':
					$child_value = $this_child->content;
				break;
	
				case 'datetime_q':
					$child_value = date('d M Y H:i:s',	iso8601_ts($this_child->content));
					if ($this_child->getAttribute('circa')) $child_value = 'circa '.$child_value;
				break;

				case 'select_q':
					$select_options =& $this_child->children;
					$child_value = Array();		// store child as an array

					for (reset($select_options); null !== ($k = key($select_options)); 	next($select_options)) {
						$this_option =& $select_options[$k];
						$child_value[] = $this_option->content;
					}
					$child_value = implode(', ', $child_value);
				break;
			}

			if ($this_child->name != 'section') {
				$children[] = Array('id' => $child_id, 'name' => $child_name, 'content' => $child_value);
			}
		}

		return $children;

	}//end _generateSubmissionList()


//--        RECEIPT FIELD        --//


	/**
	* Paints the interface for selecting a field containing send to a friend entries
	*
	* @param &object Form_Email			$asset		the form involved
	* @param &object Backend_Outputter	$o			the backend outputter
	* @param string						$prefix		a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintReceiptField(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return false;
		$assetid = $asset->attr('receipt_field');
		asset_finder($prefix.'_rec_field', $assetid, Array('form_question_type_email_address' => 'I'));

		return true;

	}//end paintReceiptField()
	

	/**
	* Processes the interface for selecting a field containing a receipt email
	*
	* @param &object Form_Email			$asset		the form involved
	* @param &object Backend_Outputter	$o			the backend outputter
	* @param string						$prefix		a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processReceiptField(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_rec_field'])) return false;
		$assetid = $_POST[$prefix.'_rec_field']['assetid'];

		// add check that this is in the same form

		// if we already have this as the not found asset, don't set it again
		if ($assetid == $asset->attr('receipt_field')) return false;
		if (!$asset->setAttrValue('receipt_field', $assetid)) return false;

		return true;

	}//end processReceiptField()


//--        SEND TO A FRIEND STUFF        --//


	/**
	* Paints the interface for selecting a field containing send to a friend entries
	*
	* @param &object Form_Email			$asset		the form involved
	* @param &object Backend_Outputter	$o			the backend outputter
	* @param string						$prefix		a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function paintSTAFField(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return false;
		$assetid = $asset->attr('staf_field');
		asset_finder($prefix.'_staf_field', $assetid, Array('form_question_type_text' => 'D'));

		return true;

	}//end paintSTAFField()


	/**
	* Processes the interface for selecting a field containing send to a friend entries
	*
	* @param &object Form_Email			$asset		the form involved
	* @param &object Backend_Outputter	$o			the backend outputter
	* @param string						$prefix		a unique prefix for variables
	*
	* @return boolean
	* @access public
	*/
	function processSTAFField(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_staf_field'])) return false;
		$assetid = $_POST[$prefix.'_staf_field']['assetid'];

		// todo: add check that this is in the same form

		// if we already have this as the not found asset, don't set it again
		if ($assetid == $asset->attr('staf_field')) return false;
		if (!$asset->setAttrValue('staf_field', $assetid)) return false;

		return true;

	}//end processSTAFField()


//--        SELECTIVE E-MAILS        --//


	/**
	* Paints the interface for creating a new Selective Email rule
	*
	* @param object Form_Email			&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddSelectiveEmailRule(&$asset, &$o, $prefix)
	{
		if(!$asset->writeAccess('attributes')) return false;
		check_box($prefix.'_new_se_rule', '1', false);
		echo ' New Rule';

		return true;

	}//end paintAddSelectiveEmailRule()


	/**
	* Processes the interface for creating a new, blank Selective Email rule
	*
	* @param object Form_Email			&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddSelectiveEmailRule(&$asset, &$o, $prefix)
	{
		if(!isset($_REQUEST[$prefix.'_new_se_rule'])) return false;
		$all_se = $asset->attr('selective_emails');

		// pull out a new index for this question
		if (empty($all_se)) {
			$new_id = 0;
		} else {
			$new_id = max(array_keys($all_se)) + 1;
		}

		// fill out the defaults
		$all_se[$new_id] = Array('assetid' => '',
								 'rules'   => Array(),
								 'require' => 'all',
								 'send'    => 'recipient',
								 'address' => '');

		// set the post/request variables so the later functions know to show the new rule
		$_REQUEST[$prefix.'_se_active_ruleid'] = $new_id;

		$_POST[$prefix.'_require']	 = 'all';
		$_POST[$prefix.'_send']		 = 'recipient';
		$_POST[$prefix.'_address']	 = '';

		if (!$asset->setAttrValue('selective_emails', $all_se)) return false;
		return $asset->saveAttributes();

	}//end processAddSelectiveEmailRule()


	/**
	* paint the current rules
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintCurrentSelectiveEmails(&$asset, &$o, $prefix)
	{
		$admin_access = $asset->adminAccess('attributes');

		$am = &$GLOBALS['SQ_SYSTEM']->am;
			
		$ruleid = isset($_REQUEST[$prefix.'_se_active_ruleid']) ? $_REQUEST[$prefix.'_se_active_ruleid'] : -1;

		if (isset($_REQUEST[$prefix.'_se_active_ruleid'])) {
			$o->addHiddenField($prefix.'_se_active_ruleid', $_REQUEST[$prefix.'_se_active_ruleid']);
		}

		$all_se = $asset->attr('selective_emails');

		if (empty($all_se)) {
			echo 'There are no Selective Email rules yet set up for this form';
			return false;
		}

		?>
		<table class="sq-backend-table">
			<tr>
				<?php
				if ($admin_access) {
					?><td align="center" width="30" class="sq-backend-table-header" style="font-weight: bold;">Edit</td><?php
				}
				?><td class="sq-backend-table-header">
					Rule
				</td>
				<td align="center" width="50" class="sq-backend-table-header" style="font-weight: bold;">Valid ?</td>
				<?php
				if ($admin_access) {
					?><td align="center" width="70" class="sq-backend-table-header" style="font-weight: bold;">Delete ?</td>
				<?php
				}
				?>
			</tr>
		<?php
		for (reset($all_se); null !== ($k = key($all_se)); next($all_se)) {
			$info = &$all_se[$k];
			$rule_prefix = $prefix.'_r'.$k;
			$valid = true;
			?>
			<tr>
				<?php
				if ($admin_access) {
					?>
					<td valign="top" align="center" width="30" class="sq-backend-table-cell<?php if ($ruleid == $k) echo '-alt' ?>"><a href="<?php echo $o->getCurrentLocation(); ?>&<?php echo $prefix?>_se_active_ruleid=<?php echo $k ?>"><img src="<?php echo sq_web_path('lib'); ?>/web/images/icons/edit_mode.png" width="15" height="15" border="0" /></a></td>
					<?php
				}
				?>
				<td valign="top" class="sq-backend-table-cell<?php if ($ruleid == $k) echo '-alt' ?>">
					<?php
					if (empty($info['rules'])) {
						?>
						<span><b>WARNING:</b> This rule contains no parts</span><br/>
						<?php
						$valid = false;
					} else {
						$rules =& $info['rules'];
		
						$i = 0;
						for (reset($rules); null !== ($rule_key = key($rules)); next($rules)) {
							$rule =& $rules[$rule_key];
							$type_code = 'form_question_rule_type_'.$rule['name'];
							$q_asset =& $am->getAsset($rule['assetid']);
		
							// get operators and attributes from a static call
							$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
							$operators = eval('return '.$type_code.'::getOperators();');
							$attributes = eval('return '.$type_code.'::getAttributes();');
		
							// if this is not our first rule part, put in AND or OR as the case may be
							if ($i > 0) {
								if ($info['require'] == 'all') {
									echo '<b>AND</b> ';
								} else {
									echo '<b>OR</b> ';
								}
							}
							
							$i++;
							echo '<b>'.$q_asset->attr('name').'</b> ';
							echo $operators[$rule['operator']];
							echo '<b>';
						
							if (!$attributes['no_comparison']) {
								// comparison question ID
								if ($cqid = $rule['comparison_question_id']) {
									$cq_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($cqid);
										echo 'Value of "'.$cq_asset->name.'" ['.$cq_asset->type().' : '.$cqid . ']';
								} else {
									echo $rule['value'];
								}
								echo '</b>';
							}
							echo '<br />';
							$i++;
						}//end for
		
					}//end if rules not empty
		
					echo 'Send a <b>';
					if ($info['send'] == 'staf') {
						echo 'Send To A Friend';
					} else if ($info['send'] == 'recipient') {
						echo 'Recipient';
					} else if ($info['send'] == 'receipt') {
						echo 'Receipt';
					} else {
						echo 'Unknown E-mail Type';
						$valid = false;
					}
		
					echo '</b> email to <b>';
					if (empty($info['address'])) {
						$valid = false;
						echo 'No Email Address Specified';
					} else {
						echo $info['address'];
					}
					echo '</b>';
					?>
				</td>
				<td valign="top" align="center" width="70" class="sq-backend-table-cell<?php if ($ruleid == $k) echo '-alt' ?>"><img src="<?php echo sq_web_path('lib'); ?>/web/images/<?php echo $valid ? 'tick' : 'cross'; ?>.gif" width="15" height="15" /></td>
				<?php
				if ($admin_access) {
					?><td valign="top"  align="center" width="50" class="sq-backend-table-cell<?php if ($ruleid == $k) echo '-alt' ?>"><?php check_box($prefix.'_deletes[]', $k); ?></td><?php
				}
				?>
			</tr>
			<?php 
		}//end for all selective emails
		?>
		</table>
		<?php

		return true;

	}//end paintCurrentSelectiveEmails()


	/**
	* Processes the interface for deleting current selective email rules
	*
	* @param object Form_Email			&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processCurrentSelectiveEmails(&$asset, &$o, $prefix)
	{
		if (!isset($_POST[$prefix.'_deletes'])) return true;
		
		$all_se = $asset->attr('selective_emails');
		$delete_ruleids = $_POST[$prefix.'_deletes'];

		// delete our selective e-mail rules
		foreach ($delete_ruleids as $ruleid) {
			unset($all_se[$ruleid]);
			if (!empty($_REQUEST[$prefix.'_se_active_ruleid'])) {
				var_export(array($ruleid, $_REQUEST[$prefix.'_se_active_ruleid']));
				if ($ruleid == $_REQUEST[$prefix.'_se_active_ruleid']) {
					unset($_REQUEST[$prefix.'_se_active_ruleid']);
				}
			}
		}

		$asset->setAttrValue('selective_emails', $all_se);
		return $asset->saveAttributes();

	}//end processCurrentSelectiveEmails()


	/**
	* Processes the interface for selecting a question for a rule part
	*
	* @param object Form_Email			&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintActiveQuestion(&$asset, &$o, $prefix)
	{
		if (!$asset->writeAccess('attributes')) return false;
		if (isset($_REQUEST[$prefix.'_se_active_ruleid'])) {
			$current_assetid = isset($_POST[$prefix.'_se_new_qid']['assetid']) ? $_POST[$prefix.'_se_new_qid']['assetid'] : 0;
			asset_finder($prefix.'_se_new_qid', $current_assetid, Array('form_question' => 'D'));
			return true;
		}

	}//end paintActiveQuestion()


	/**
	* Processes the interface for selecting active question (no implementation required)
	*
	* @param object Form_Email			&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processActiveQuestion(&$asset, &$o, $prefix)
	{
		return true;

	}//end processActiveQuestion()


	/**
	* Paint the dropdowns to add rule parts to a Selective Email rule
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintAddRulePart(&$asset, &$o, $prefix)
	{
		if(!$asset->adminAccess('attributes')) return false;
		if (!isset($_REQUEST[$prefix.'_se_active_ruleid'])) return false;

		if (!isset($_POST[$prefix.'_se_new_qid']['assetid'])) {
			echo 'You must select a question before you can select the type of rule part';
			return false;
		} else {
			$active_question = &$GLOBALS['SQ_SYSTEM']->am->getAsset($_POST[$prefix.'_se_new_qid']['assetid']);

			$rule_codes = $active_question->getAllowedRules();
			$type_codes = Array();
			foreach	($rule_codes as $rule_code) $type_codes[] = 'form_question_rule_type_'.$rule_code;

			$desc = $GLOBALS['SQ_SYSTEM']->am->getTypeInfo($type_codes, 'description');
			combo_box($prefix.'_new_rule', array_merge(Array(' ' => '------ Please Select ------'), $desc), false, '');
		
			return true;
		}

	}//end paintAddRulePart()
	
	
	/**
	* Process the interface for adding a rule part to a Selective Email rule
	*
	* @param object Form_Section		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processAddRulePart(&$asset, &$o, $prefix)
	{
		if (!$asset->adminAccess('attributes')) return false;

		// rule id is not set || required "new rule part" fields not set = FAIL
		if (!isset($_REQUEST[$prefix.'_se_active_ruleid']) ||
			!isset($_REQUEST[$prefix.'_new_rule']) || empty($_POST[$prefix.'_se_new_qid']['assetid'])) {
			return false;

		} else {
			$ruleid = $_REQUEST[$prefix.'_se_active_ruleid'];
			$qid = $_POST[$prefix.'_se_new_qid']['assetid'];
			$active_question = &$GLOBALS['SQ_SYSTEM']->am->getAsset($qid);

			$type_code = $_POST[$prefix.'_new_rule'];
			$rule_code = substr($type_code,24);
			if (!trim($rule_code)) return false;

			$all_se = $asset->attr('selective_emails');

			$rules =& $all_se[$ruleid]['rules'];
		
			$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
			$operators = array_keys(eval('return '.$type_code.'::getOperators();'));

			$rules[] = Array('assetid'				=> $qid,
						   'name'					=> $rule_code,
						   'operator'				=> $operators[0],
						   'value'					=> '',
						   'comparison_question_id'	=> 0);
			

			$asset->setAttrValue('selective_emails', $all_se);
			return $asset->saveAttributes();
		}

	}//end processAddRulePart()


	/**
	* Paints the interface for editing and deleting Rule Parts
	*
	* @param object Form_Email			&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function paintRuleParts(&$asset, &$o, $prefix)
	{
		if (!isset($_REQUEST[$prefix.'_se_active_ruleid'])) return false;
		$ruleid = $_REQUEST[$prefix.'_se_active_ruleid'];

		if (!$asset->writeAccess('attributes')) return false;
		$admin_access = $asset->adminAccess('attributes');

		$am		=& $GLOBALS['SQ_SYSTEM']->am;
		$all_se	=& $asset->attr('selective_emails');
		$rules  =& $all_se[$ruleid]['rules'];

		if (empty($rules)) {
			echo 'This rule is empty';
			return false;
		}

		// set a get var so that we know what question was selected when we come back 
		?>
		<table class="sq-backend-table">
			<tr>
				<td class="sq-backend-table-header">
					Question
				</td>
				<td class="sq-backend-table-header">
					Rule
				</td>
				<td class="sq-backend-table-header">
					Comparison Value
				</td>
				<td align="center" width="70" class="sq-backend-table-header">
					Delete ?
				</td>
			</tr>

			<?php
			// sort order is based upon shadow asset id (eg. '87:q1'), not just question id - this 	
			// will allow sections to be sorted in later
			$i = 0;

			for (reset($rules); null !== ($k = key($rules)); next($rules)) {
				$rule = &$rules[$k];
				$type_code = 'form_question_rule_type_'.$rule['name'];
				$q_asset =& $am->getAsset($rule['assetid']);

				// get operators and attributes from a static call
				$GLOBALS['SQ_SYSTEM']->am->includeAsset($type_code);
				$operators = eval('return '.$type_code.'::getOperators();');
				$attributes = eval('return '.$type_code.'::getAttributes();');

				// obtain a rule prefix
				$rule_prefix = $prefix.'_r'.$k;
				?>
				<td valign="top" class="sq-backend-table-cell">
				<?php echo $q_asset->attr('name'); ?>
				</td>
				<td class="sq-backend-table-cell">
					<?php if ($asset->writeAccess('attributes')) {
						combo_box($rule_prefix.'_operator', $operators, 
						false, $rule['operator']); 
					} else {
						echo $operators[$rule['operator']];
					} ?>
				</td>
				<td class="sq-backend-table-cell">
					<?php if (!$attributes['no_comparison']) {
						// only print if we need to compare with something
						if ($asset->writeAccess('attributes')) {
							if ($q_asset->isSelection() && !$attributes['force_text']) {
								$options = $asset->getOptions();
								combo_box($rule_prefix.'_value', array_merge(Array(' ' => '------ Please Select ------'), $options), 
						false, $rule['value']); 
							} else {
								text_box($rule_prefix.'_value', $rule['value'], 30, 255);
							}
							// showing the asset finder?
							if ($attributes['show_asset_finder']) {
								echo ' Or<br />';
								asset_finder($rule_prefix.'_cq_id', $rule['comparison_question_id'], Array('form_question' => 'D'));
							}
						} else {
							// comparison question ID
							if ($cqid = $rule['comparison_question_id']) {
								$cq_asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($cqid);
								echo 'Value of "'.$cq_asset->name.'" ['.$cq_asset->type().' : '.$cqid . ']';
							} else {
								echo $rule['value'];
							}
						}
					}?>
				</td>
				<?php
				if ($admin_access) {
					?><td valign="top"  align="center" width="70" class="sq-backend-table-cell"><?php check_box($prefix.'_rule_deletes[]', $k); ?></td><?php
				} 
				?>
				</tr>
				<?php
				$i++;

			}//end for all rules
		?>
		</table>
		<?php

		return true;

	}//end paintRuleParts()


	/**
	* Processes the interface for editing and deleting Rule Parts
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processRuleParts(&$asset, &$o, $prefix)
	{
		$all_se = $asset->attr('selective_emails');
		if (!isset($_REQUEST[$prefix.'_se_active_ruleid'])) return false;

		$ruleid = $_REQUEST[$prefix.'_se_active_ruleid'];
		if (!isset($all_se[$ruleid])) return false;

		$rules =& $all_se[$ruleid]['rules'];

		$prefix = $asset->getPrefix();

		if (!empty($_POST[$prefix.'_rule_deletes'])) {
			// delete our selective e-mail rules
			$delete_ruleids = $_POST[$prefix.'_rule_deletes'];
			foreach ($delete_ruleids as $deleted_ruleid) unset($rules[$deleted_ruleid]);
		}

		// modify what's left
		for (reset($rules); null !== ($k = key($rules)); next($rules)) {
			$rule = &$rules[$k];
			$type = 'form_question_rule_type_'.$rule['name'];
			$rule_prefix = $prefix.'_r'.$k;

			if (isset($_POST[$rule_prefix.'_operator'])) {
				$rule['operator'] = $_POST[$rule_prefix.'_operator'];
			}

			if (isset($_POST[$rule_prefix.'_cq_id']) && ($_POST[$rule_prefix.'_cq_id']['assetid'] != 0)) {
				// comparison question id overrides the static value!
				if ($_POST[$rule_prefix.'_cq_id']['assetid'] == $asset->id) {
					trigger_error('Cannot compare "'.$asset->name.'" (# '.$asset->id.') with itself in a rule',  E_USER_WARNING);
					return false;
				}
				$rule['comparison_question_id'] = $_POST[$rule_prefix.'_cq_id']['assetid'];
				$rule['value'] = '';
			} else if (!empty($_POST[$rule_prefix.'_value'])) {
				// static value only specified - set c.q. ID to a duck
				$rule['comparison_question_id'] = 0;
				$rule['value'] = $_POST[$rule_prefix.'_value'];
			}
		}//end for

		$asset->setAttrValue('selective_emails', $all_se);
		return $asset->saveAttributes();

	}//end processRuleParts()


	/**
	* Paint the dropdowns to add rules to this question
	*
	* @param object Form_Section		&$asset		the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o			the outputter class
	* @param string						$prefix		prefix for the form elements
	*
	* @return boolean false if no questions in this section, true otherwise
	* @access public
	*/
	function paintActiveSend(&$asset, &$o, $prefix)
	{
		if (!$asset->adminAccess('attributes')) return false;
		if (!isset($_REQUEST[$prefix.'_se_active_ruleid'])) return false;

		$ruleid = $_REQUEST[$prefix.'_se_active_ruleid'];
		$all_se = $asset->attr('selective_emails');

		echo 'When ';
		combo_box($prefix.'_require', Array('all' => 'ALL', 'any' => 'ANY'), false, Array($all_se[$ruleid]['require']));
		echo ' rule parts are matched,<br/>send a ';
		combo_box($prefix.'_send', Array('recipient' => 'Recipient E-mail', 'receipt' => 'Receipt', 'staf' => 'Send To A Friend E-mail'), false, Array($all_se[$ruleid]['send']));
		echo ' to ';
		text_box($prefix.'_address', $all_se[$ruleid]['address'], 30, 255);
		
		return true;

	}//end paintActiveSend()
	
	
	/**
	* Process the newly added questions
	*
	* @param object Form_Section		&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processActiveSend(&$asset, &$o, $prefix)
	{
		if(!$asset->adminAccess('attributes')) return false;
		if (!isset($_REQUEST[$prefix.'_se_active_ruleid']) || !isset($_POST[$prefix.'_address'])) return false;

		$ruleid = $_REQUEST[$prefix.'_se_active_ruleid'];
		$all_se = $asset->attr('selective_emails');

		$all_se[$ruleid]['require'] = $_POST[$prefix.'_require'];
		$all_se[$ruleid]['send']    = $_POST[$prefix.'_send'];
		$all_se[$ruleid]['address'] = $_POST[$prefix.'_address'];

		$asset->setAttrValue('selective_emails', $all_se);
		return $asset->saveAttributes();

	}//end processActiveSend()


}//end class
?>
