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
* $Id: report_broken_links_edit_fns.inc,v 1.3 2004/07/06 05:08:22 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/


require_once SQ_CORE_PACKAGE_PATH.'/report/report_edit_fns.inc';

/**
* Report_Broken_Links_Edit_Fns
*
* Purpose
*
*
* @author  Greg Sherwood <greg@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix_Packages
* @subpackage cms
*/
class Report_Broken_Links_Edit_Fns extends Report_Edit_Fns
{


	/**
	* Constructor
	*
	*/
	function Report_Broken_Links_Edit_Fns()
	{
		$this->Report_Edit_Fns();

	}//end constructor


	/**
	* Generate this report until complete
	*
	* @param object HIPO_Job	&$job		a reference to the array of information about
	* @param array				&$step_data	a reference to the array of information about
	*										the current step of the HIPO job that is running this report
	* @param string				$prefix		prefix for form vars
	*
	* @return boolean
	* @access public
	*/
	function generateReport(&$job, &$step_data, $prefix)
	{
		require_once SQ_FUDGE_PATH.'/general/text.inc';
		require_once SQ_FUDGE_PATH.'/general/file_system.inc';
		$owner = &$GLOBALS['SQ_SYSTEM']->am->getAsset($job->_running_vars['assetid'], 'report_broken_links');

		if (!isset($job->_running_vars['results'])) {
			$job->_running_vars['todo'] = $GLOBALS['SQ_SYSTEM']->am->getChildren($this->getRootAssetid($owner), 'content_type_wysiwyg', false);
			$job->_running_vars['done'] = Array();
			$job->_running_vars['results'] = Array();

			if (!is_dir($owner->data_path)) {
				if (!create_directory($owner->data_path)) {
					trigger_error('Could not create data directory, report generation aborted', E_USER_WARNING);
					return false;
				}
			}
			$temp_file = fopen($owner->data_path.'/report.tmp', 'w');
			if ($temp_file === false) {
				trigger_error('Could not open temporary file, report generation aborted', E_USER_WARNING);
				return false;
			}
			fwrite($temp_file, "<links>\n");
		} else {
			$temp_file = fopen($owner->data_path.'/report.tmp', 'a');
			if ($temp_file === false) {
				trigger_error('Could not open temporary file, report generation aborted', E_USER_WARNING);
				return false;
			}
		}

		if (!empty($job->_running_vars['todo'])) {

			$assetid = array_shift(array_keys($job->_running_vars['todo']));
			$asset_type = $job->_running_vars['todo'][$assetid];
			unset($job->_running_vars['todo'][$assetid]);
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, $asset_type);

			if (!is_null($asset)) {
				$step_data['message'] = 'Generating broken links report...';
				
				$wysiwyg_html = $asset->attr('html');
				
				$master_matches = Array();
				preg_match_all('%(href|src)=[\'"]([^\'\s"]*)%', $wysiwyg_html, $master_matches);
				$matches = $master_matches[2];
			
				foreach ($matches as $match) {
					// skip keywords
					$keywords = retrieve_keywords_replacements($match);
					if (count($keywords) == 1 && $match == '%'.$keywords[0].'%') continue;
					
					if (!$match) continue;
					$link_valid = '0';
					$link_description = '';
					$link_type = '';

					$results = Array();
					preg_match('|\./\?a=([0-9]+)|',$match, $results);

					$validated_results = false;
					if (!empty($results)) {
						$link_type = 'internal';
						$validated_results = $this->validateInternalLink($results[1]);
					} else {
						if (strpos(strtolower($match), 'mailto') === 0) {
							$link_type = 'mailto';
							$match = preg_replace('/mailto:/i','',$match);
							list($match) = explode('?',$match);
							$match = trim($match);
							$validated_results = $this->validateEmailLink($match);
						} else {
							$link_type = 'external';
							$validated_results = $this->validateExternalLink($match);
						}
					}
					
					if ($validated_results === false) continue;
					
					fwrite($temp_file, "\t<link assetid=\"".$asset->id."\" type=\"$link_type\" valid=\"".$validated_results['valid']."\">\n");
					fwrite($temp_file, "\t\t<html>$match</html>\n");
					fwrite($temp_file, "\t\t<description>".$validated_results['description']."</description>\n");
					fwrite($temp_file, "\t</link>\n");
				}//end foreach

				$GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);
				unset($asset);
			} else {
				$step_data['message'] = 'Skipping asset '.$assetid;
				$job->_addError('Could not generate broken links report for assetd #'.$assetid.', asset does not exist');
			}

			// add this assetid to the done array so we dont do it again
			$job->_running_vars['done'][] = $assetid;
		}

		if (empty($job->_running_vars['todo'])) {
			$step_data['percent_done'] = 100;
			$step_data['complete']     = true;

			// move the temp file to become the real report XML file
			fwrite($temp_file, "</links>\n");
			if (!copy($owner->data_path.'/report.tmp', $owner->data_path.'/report.xml')) {
				trigger_error('Could not move temporary file, report generation aborted', E_USER_WARNING);
				return false;
			}
			if (!unlink($owner->data_path.'/report.tmp')) {
				trigger_error('Could not delete temporary file, report generation continues', E_USER_WARNING);
			}
		} else {
			$total = count($job->_running_vars['todo']) + count($job->_running_vars['done']);
			$step_data['percent_done'] = (count($job->_running_vars['done']) / $total) * 100;
			$step_data['complete']     = false;
		}

		fclose($temp_file);
		return true;

	}//end generateReport()


	/**
	* Check the validity of an internal link
	*
	* @param string	$assetid	the id of the asset being linked to
	*
	* @return array
	* @access public
	*/
	function validateInternalLink($assetid)
	{
		$asset = null;
		if ($assetid) $asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, '', true);

		if (is_null($asset)) {
			// the asset was invalid
			$description = 'Link points to an invalid assetid';
			$valid = '0';
		} else if ($GLOBALS['SQ_SYSTEM']->am->assetInTrash($assetid, true)) {
			// the asset is in the trash
			$description = 'Link points to an asset that is in the trash';
			$valid = '0';
		} else {
			$valid = '1';
			$description = '';
		}
		if (!is_null($asset)) $GLOBALS['SQ_SYSTEM']->am->forgetAsset($asset);

		return Array('valid' => $valid, 'description' => $description);

	}//end validateInternalLink()
	
	
	/**
	* Check the validity of an external link
	*
	* @param string	$url	the url being validated
	*
	* @return array
	* @access public
	*/
	function validateExternalLink($url)
	{
		// ignore bookmarks
		if (substr($url, 0, 1) == '#') return false;
		
		$url_parts = @parse_url($url);
		if (empty($url_parts['scheme'])) {
			$description = 'The URL is missing a protocol';
			$valid = '0';
		} else if (empty($url_parts['host'])) {
			$description = 'The URL is missing a hostname';
			$valid = '0';
		} else if ($url_parts['scheme'] != 'http') {
			$description = 'The URL uses a protocol that cannot be tested';
			$valid = '1';
		} else {
			if (empty($url_parts['path'])) $url_parts['path'] = '/';
			if (empty($url_parts['port'])) $url_parts['port'] = '80';
			
			$url_path = $url_parts['path'];
			if (!empty($url_parts['query'])) $url_path .= '?'.$url_parts['query'];

			$socket = @fsockopen($url_parts['host'], $url_parts['port'], $errno, $errstr, 15);
			if (!$socket) {
				$description = 'This URL does not appear to be valid';
				$valid = '0';
			} else {
				fwrite ($socket, "HEAD ".$url_path." HTTP/1.0\r\nHost: ".$url_parts['host']."\r\n\r\n");
				$response = fgets($socket, 128);
				$headers = '';
				while (!feof($socket)) $headers .= fgets($socket, 128);

				if (preg_match('|200 ok|i', $response)) {
					$description = '';
					$valid = '1';
				} else if (preg_match('|404 not found|i', $response)) {
					$description = 'The URL was not found';
					$valid = '0';
				} else if (preg_match('|301 moved|i', $response)) {
					$description = 'The URL has been moved permanently';
					$valid = '1';
					preg_match('|Location:\s*(.+)\r|i', $headers, $matches);
					if (isset($matches[1])) $description .= ' to "'.$matches[1].'"';
				} else {
					$description = 'The URL returned response "'.$response.'"';
					$valid = '0';
				}
				fclose($socket);
			}
		}

		return Array('valid' => $valid, 'description' => $description);

	}//end validateExternalLink()
	
	
	/**
	* Check the validity of an mailto
	*
	* @param string	$url	the mailto being validated
	*
	* @return array
	* @access public
	*/
	function validateEmailLink($mailto)
	{
		require_once SQ_FUDGE_PATH.'/general/www.inc';
		if (!valid_email($mailto)) {
			$description = 'The email address does not appear to be valid';
			$valid = '0';
		} else {
			$description = '';
			$valid = '1';
		}

		return Array('valid' => $valid, 'description' => $description);

	}//end validateEmailLink()


	/**
	* Paint the interface for viewing a report
	*
	* @param object Report				$owner	the asset whose interface we are painting
	* @param object	Backend_Outputter	$o		the outputter class
	* @param string						$prefix	prefix for the form element
	*
	* @return boolean
	* @access public
	*/
	function paintReport(&$asset, &$o, $prefix)
	{
		require_once 'XML/Tree.php';
		require_once SQ_INCLUDE_PATH.'/general_occasional.inc';
		require_once SQ_FUDGE_PATH.'/general/general.inc';

		$report_path = $asset->data_path.'/report.xml';
		if (!is_file($report_path)) {
			echo 'This report has not been generated';
			return;
		}

		$input = new XML_Tree($report_path);
		$root  = &$input->getTreeFromFile();
		if (PEAR::isError($root)) {
			trigger_error('Unable to read report file : '.$report_path."\n".$root->getMessage()."\n".$root->getUserInfo(), E_USER_WARNING);
			return;
		}
		
		$reported_links = Array( 'internal' => Array(),
								 'external' => Array(),
								 'mailto'   => Array(),
								);
		

		foreach ($root->children as $link) {
			if ($link->name != 'link') continue;
			assert_isset_array_index($reported_links, $link->attributes['type'], 'Unkown link type "'.$link->attributes['type'].'"');

			$new_link = Array();
			foreach ($link->children as $link_child) {
				switch ($link_child->name) {
					case 'html' :
						$new_link['html'] = $link_child->content;
						
						// get the assetid from the html link
						if ($link->attributes['type'] == 'internal') {
							$to_assetid = preg_replace('|\./\?a=([0-9]+)|', '\\1', $link_child->content);
							$new_link['data'] = $to_assetid;
						} else {
							$new_link['data'] = $link_child->content;
						}
					break;
					case 'description' :
						$new_link['description'] = $link_child->content;
					break;
				}
			}//end foreach
			
			$new_link['valid'] = $link->attributes['valid'];
			$reported_links[$link->attributes['type']][$link->attributes['assetid']][] = $new_link;
		}//end foreach


		echo '<b>Note</b> The current broken links report is shown below. You can regenerate this report on the details screen.';
		$o->closeSection();


		$o->openSection('Internal Links');
			$o->openField('');
				if (empty($reported_links['internal'])) {
					echo 'No internal links were found';
				} else {
					?>
					<table class="sq-backend-table">
						<tr>
							<td colspan="2" class="sq-backend-table-header" style="width: 266px;">Found In</td>
							<td colspan="2" class="sq-backend-table-header" style="width: 266px;">Linking To</td>
							<td class="sq-backend-table-header">Description</td>
						</tr>
					<?php
					foreach ($reported_links['internal'] as $assetid => $links) {
						foreach ($links as $link) {
							$from_info = $this->_getAssetReportInfo($assetid);
	
							$to = null;
							if ($link['data']) {
								$to = &$GLOBALS['SQ_SYSTEM']->am->getAsset($link['data'], '', true);
							}
							if (is_null($to)) {
								$to_type = '';
								$to_string = 'Unkown asset #'.$link['data'];
								$to_bg = 'FFFFFF';
							} else {
								$to_type = $to->type();
								$to_string = $to->name;
								$to_bg = get_status_colour($to->status);
							}
							?>
							<tr>
								<td class="sq-backend-table-cell" style="border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; width: 16px; background-color: #<?php echo $from_info['bg']; ?>;">
									<?php
									if (!empty($from_info['type'])) {
										?>
										<script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($from_info['type']); ?>", "16", "16", "");</script>
										<?php
									}
									?>
								</td>
								<td class="sq-backend-table-cell" style="border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-right: 1px solid #000000; width: 250px; background-color: #<?php echo $from_info['bg']; ?>;">
									<?php echo $from_info['string']; ?>
								</td>
								<td class="sq-backend-table-cell" style="border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; width: 16px; background-color: #<?php echo $to_bg; ?>;">
									<?php
									if (!empty($to_type)) {
										?>
										<script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($to_type); ?>", "16", "16", "");</script>
										<?php
									}
									?>
								</td>
								<td class="sq-backend-table-cell" style="border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-right: 1px solid #000000; width: 250px; background-color: #<?php echo $to_bg; ?>;">
									<?php
									if (!empty($to_type)) {
										?>
										<acronym title="<?php echo ucwords(str_replace('_', ' ', $to_type)).' [#'.$to->id.']'; ?>"><?php echo $to_string; ?></acronym>
										<?php
									} else {
										echo $to_string;
									}
									?>
								</td>
								<td class="sq-backend-table-cell" style="border: 1px solid black;">
									<?php
									if (!$link['valid']) echo '<span class="sq-backend-warning"><b>';
									echo $link['description'];
									if (!$link['valid']) echo '</b></span>';
									?>
								</td>
							</tr>
							<?php
						}//end foreach link
					}//end foreach asset links
					?>
					</table>
					<?php
				}//end if
			$o->closeField();
		$o->closeSection();


		$o->openSection('External Links');
			$o->openField('');
				if (empty($reported_links['external'])) {
					echo 'No external links were found';
				} else {
					?>
					<table class="sq-backend-table">
						<tr>
							<td colspan="2" class="sq-backend-table-header" style="width: 266px;">Found In</td>
							<td class="sq-backend-table-header" style="width: 274px;">URL</td>
							<td class="sq-backend-table-header">Description</td>
						</tr>
					<?php
					foreach ($reported_links['external'] as $assetid => $links) {
						foreach ($links as $link) {
							$from_info = $this->_getAssetReportInfo($assetid);
							
							$to_string = '<a href="'.$link['data'].'" target="_blank" title="'.$link['data'].'">'.ellipsisize($link['data'], 50).'</a>';
							?>
							<tr>
								<td class="sq-backend-table-cell" style="border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; width: 16px; background-color: #<?php echo $from_info['bg']; ?>;">
									<?php
									if (!empty($from_info['type'])) {
										?>
										<script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($from_info['type']); ?>", "16", "16", "");</script>
										<?php
									}
									?>
								</td>
								<td class="sq-backend-table-cell" style="border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-right: 1px solid #000000; width:250px; background-color: #<?php echo $from_info['bg']; ?>;">
									<?php echo $from_info['string']; ?>
								</td>
								<td class="sq-backend-table-cell" style="width: 274px; border: 1px solid #000000;">
									<?php echo $to_string; ?>
								</td>
								<td class="sq-backend-table-cell" style="border: 1px solid #000000;">
									<?php
									if (!$link['valid']) echo '<span class="sq-backend-warning"><b>';
									echo $link['description'];
									if (!$link['valid']) echo '</b></span>';
									?>
								</td>
							</tr>
							<?php
						}//end foreach link
					}//end foreach asset links
					?>
					</table>
					<?php
				}//end if
			$o->closeField();
		$o->closeSection();


		$o->openSection('Email Links');
			$o->openField('');
				if (empty($reported_links['mailto'])) {
					echo 'No email links were found';
				} else {
					?>
					<table class="sq-backend-table">
						<tr>
							<td colspan="2" class="sq-backend-table-header" style="width: 266px;">Found In</td>
							<td class="sq-backend-table-header" style="width: 274px;">Email</td>
							<td class="sq-backend-table-header">Description</td>
						</tr>
					<?php
					foreach ($reported_links['mailto'] as $assetid => $links) {
						foreach ($links as $link) {
							$from_info = $this->_getAssetReportInfo($assetid);
							$to_string = $link['data'];
							?>
							<tr>
								<td class="sq-backend-table-cell" style="border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-left: 1px solid #000000; width: 16px; background-color: #<?php echo $from_info['bg']; ?>;">
									<?php
									if (!empty($from_info['type'])) {
										?>
										<script language="JavaScript" type="text/javascript">sq_print_icon("<?php echo $GLOBALS['SQ_SYSTEM']->am->getAssetIconURL($from_info['type']); ?>", "16", "16", "");</script>
										<?php
									}
									?>
								</td>
								<td class="sq-backend-table-cell" style="border-top: 1px solid #000000; border-bottom: 1px solid #000000; border-right: 1px solid #000000; width: 250px; background-color: #<?php echo $from_info['bg']; ?>;">
									<?php echo $from_info['string']; ?>
								</td>
								<td class="sq-backend-table-cell" style="width: 274px; border: 1px solid #000000;">
									<?php echo $to_string; ?>
								</td>
								<td class="sq-backend-table-cell" style="border: 1px solid #000000;">
									<?php
									if (!$link['valid']) echo '<span class="sq-backend-warning"><b>';
									echo $link['description'];
									if (!$link['valid']) echo '</b></span>';
									?>
								</td>
							</tr>
							<?php
						}//end foreach link
					}//end foreach asset links
					?>
					</table>
					<?php
				}//end if
			$o->closeField();
		$o->closeSection();

	}//end paintReport()
	
	
	/**
	* Get reporting information about an asset
	*
	* @param string	$assetid	the id of the asset we are getting reporting info for
	*
	* @return array('assetid', 'type', 'string', 'bg')
	* @access private
	*/
	function _getAssetReportInfo($assetid)
	{
		$asset = null;
		if ($assetid) {
			$asset = &$GLOBALS['SQ_SYSTEM']->am->getAsset($assetid, '', true);
		}
		if (is_null($asset)) {
			$asset_type = '';
			$asset_string = 'Unkown asset #'.$assetid;
			$asset_bg = 'FFFFFF';
		} else {
			$dependants = $GLOBALS['SQ_SYSTEM']->am->getDependantParents($asset->id);
			$parent = $GLOBALS['SQ_SYSTEM']->am->getAsset(array_pop($dependants));
			if (!is_null($parent)) {
				$asset_type = $parent->type();
				$asset_string = $parent->name;
				$asset_bg = get_status_colour($parent->status);
				$assetid = $parent->id;
				$asset_url = $parent->getBackendHref();
			} else {
				$asset_type = $asset->type();
				$asset_string = $asset->name;
				$asset_bg = get_status_colour($asset->status);
				$asset_url = $asset->getBackendHref();
			}
			
			$asset_string = '
				<a href="'.$asset_url.'" title="'.ucwords(str_replace('_', ' ', $asset_type)).' [#'.$assetid.']">
					'.$asset_string.'
				</a>';
		}

		return Array('assetid' => $assetid, 'type' => $asset_type, 'string' => $asset_string, 'bg' => $asset_bg);

	}//end _getAssetReportInfo()


}//end class

?>
