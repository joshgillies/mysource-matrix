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
* $Id: form_email_edit_fns.inc,v 1.15 2004/05/17 06:44:15 lwright Exp $
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
	}//end Form_Email_Edit_Fns()


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
	function paintSubmissions(&$asset, &$o, $prefix) {
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
					<?php // display only if expanded
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
								foreach($elements as $element) {
									$i++;
									if (($i % 2 == 1)) { ?>
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
								
								}	// end-foreach section element

								?></tr><?php
								
							} // end-if this element is a section

						} // end-foreach root element

					} // end-if this element expanded

				} // end-if file's datestamp in range

			} // end-while reading directories

		} // end-handle opened
		closedir($handle);
		?>
		</table>
		<?php

	}//end paintSubmissions()


	/**
	* processes the submissions block (nothing to process!)
	*
	* @param object Form_Email			&$asset	the asset whose interface we are painting
	* @param object	Backend_Outputter	&$o		the outputter class
	* @param string						$prefix	prefix for the form elements
	*
	* @return boolean
	* @access public
	*/
	function processSubmissions(&$asset, &$o, $prefix) {
		return true;

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
	function paintExport(&$asset, &$o, $prefix) {
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
	function processExport(&$asset, &$o, $prefix) {
		if (!$_POST[$prefix.'_export_switch']) return true;
		$export_type = isset($_POST[$prefix.'_export_format']) ? $_POST[$prefix.'_export_format'] : '';
		switch($export_type) {
			case 'csv':
				$this->processExportCSV(&$asset, &$o, $prefix);
				break;

			case 'xml':
				$this->processExportXML(&$asset, &$o, $prefix);
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
	function processExportCSV(&$asset, &$o, $prefix) {
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
								
						} // end-if this element is a section

					} // end-foreach root element

				} // end-if file's datestamp in range

			} // end-while reading directories

		} // end-handle opened
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
	function processExportXML(&$asset, &$o, $prefix) {
		header('Content-Type: application/xml');

		// Defining the file as attachment forces most browsers to offer it for download
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

		echo '<submissions>';			// opening tag

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

				} // end-if file's datestamp in range

			} // end-while reading directories

		} // end-handle opened
		closedir($handle);
		
		echo '</submissions>';				// closing tag
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
	function &_generateSubmissionList(&$tree_root) {
		// initialise array
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


}//end class
?>
