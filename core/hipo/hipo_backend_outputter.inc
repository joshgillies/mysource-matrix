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
* $Id: hipo_backend_outputter.inc,v 1.5 2003/11/26 00:51:08 gsherwood Exp $
* $Name: not supported by cvs2svn $
*/

require_once SQ_SYSTEM_ROOT.'/core/include/backend_outputter.inc';


/**
* Backend_Outputter
*
* Purpose
*
*    similar to the standard backend outputter, but colour scheme modified to 
*    suite hipo jobs
*
* @author  Marc McIntyre <mmcintyre@squiz.net>
* @version $Version$ - 1.0
* @package MySource_Matrix
*/
class Hipo_Backend_Outputter extends Backend_Outputter
{

	var $_form_data = Array();

	/**
	* Constructor
	*
	* @param string	$type_code
	*
	* @access  public
	*/
	function Hipo_Backend_Outputter()
	{
		$this->Backend_Outputter();

	}//end constructor


	/**
	* Set the form name and url to submit to
	*/
	function setFormData($form_name, $url)
	{
		$this->_form_data = Array('form_name' => $form_name, 'url' => $url);

	}//end setFormData()


	/**
	* Paint the header of the page
	*
	* @access private
	*/
	function _paintHeader()
	{
		if (!headers_sent()) {
			if($this->_charset) {
				header("Content-type: text/html; charset=$this->_charset");
			} else {
				$default_charset = 'iso-8859-1';
				header("Content-type: text/html; charset=$default_charset");
			}
		}
		?>
		<html>
			<head>
				<title>HIPO Processing</title>
				<?php

				$this->_paintCssInclude($this->filesPath('css/edit.css'));
				$this->_paintCssInclude($this->filesPath('css/hipo.css'));
				foreach($this->_css_includes as $file) $this->_paintCssInclude($file);

				$this->_paintJsInclude(sq_web_path('lib').'/html_form/html_form.js');
				$this->_paintJsInclude(sq_web_path('lib').'/js/general.js');
				$this->_paintJsInclude(sq_web_path('lib').'/js/debug.js');
				$this->_paintJsInclude(sq_web_path('lib').'/js/edit.js');
				foreach($this->_js_includes as $file) $this->_paintJsInclude($file);
				?>
				<script language="Javascript" type="text/javascript">
				<!--
					var ON_LOAD_TIME_OUT = null;
					function page_on_load() {
						<?php
						foreach($this->_on_load_calls as $call) {
							echo "\n".$call;
						}//end foreach
						?>
					}// end page_on_load()

				// -->
				</script>
			</head>
			<body bgcolor="#FFFFFF" onLoad="javascript: page_on_load();">
				<table border="0" cellspacing="0" cellpadding="0" width="100%">
					<tr>
						<td><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="1" height="20" /></td>
					</tr>
					<tr>
						<td align="center">
						<form name="<?php echo $this->_form_data['form_name']; ?>" method="post" action="<?php echo $this->_form_data['url']; ?>">
					<?php
					foreach($this->_hidden_fields as $name => $value) hidden_field($name, $value);

	}//end _paintHeader()


	/**
	* Paint the footer of the page
	*
	* @access private
	*/
	function _paintFooter()
	{
		?>
					</form>
					</td>
				</tr>
			</table>
			</body>
		</html>
		<?php

	}//end _paintFooter()


	/**
	* Paint the passed section onto the page
	*
	* @access private
	*/
	function _paintSection($section, $depth=0)
	{
		if ($section['type'] == 'raw') {
			echo $section['contents'];
		} else {

		?>
			<a name="section_<?php echo $section['section_count']; ?>"></a>
			<table width="100%" cellpadding="0" cellspacing="0" border="0">
			<?php
			if ($section['heading']) {
				if ($depth > 0) {
					// printing a nested section - needs to look a little different
					?>
					<tr>
						<td class="sq-backend-section-subheading" colspan="3"><?php echo $section['heading'];?></td>
					</tr>
					<?php
				} else {
					// printing a top level section
					?>
					<tr>
						<td colspan="3">
							<table border="0" cellspacing="0" cellpadding="0" width="100%">
								<tr>
									<td class="sq-backend-section-heading" width="100%"><?php echo $section['heading'];?></td>
									<td><img src="<?php echo $this->filesPath('images/section_icon.gif');?>" width="27" height="21" /></td>
								</tr>
							</table>
						</td>
					</tr>
					<tr>
						<td colspan="3">
							<table width="100%" cellpadding="0" cellspacing="0" border="0">
								<tr>
									<td width="100%" bgcolor="#B3B3B3"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="1" height="1" /></td>
								</tr>
							</table>
						</td>
					</tr>
					<?php
				}
			} // endif

			// print the section seperator
			?>
			<tr>
				<td colspan="3"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="1" height="10" /></td>
			</tr>
			<tr>
				<td><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="10" height="1" /></td>
				<td width="100%">
					<table cellspacing="0" cellpadding="0" border="0" width="100%">
			<?php

			for($j = 0; $j < count($section['areas']); $j++) {
				switch($section['areas'][$j]['area_type']) {
					case 'section' :
						?>
							<tr>
								<td width="100%" valign="top" colspan="3">
								<?php
									$this->_paintSection($section['areas'][$j], $depth + 1);
								?>
								</td>
							</tr>
						<?php

						break;

					case 'field' :
						$field = &$section['areas'][$j];
						switch ($field['format']) {
							case 'new_line' :
								?>
									<tr>
										<td nowrap valign="top" class="sq-backend-field" colspan="3">
											<a name="field_<?php echo $field['field_count']; ?>"></a>
											<?php echo $field['name'];?>
										</td>
									</tr>
									<tr>
										<td valign="top" class="sq-backend-data" colspan="3">
											<table cellpadding="0" cellspacing="0" border="0">
												<tr>
													<td><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="5" height="1"/></td>
													<td width="100%" valign="top" class="sq-backend-data">
													<?php
														echo $field['contents'];
														if ($field['note']) $this->note($field['note']);
													?>
													</td>
												</tr>
											</table>
										</td>
									</tr>
								<?php
							break;

							case 'commit' :
								?>
								<tr>
									<td width="100%" valign="top" class="sq-backend-data" style="text-align: right;" colspan="3">
									<?php
										echo $field['contents'];
										if ($field['note']) $this->note($field['note']);
									?>
									</td>
								</tr>
								<?php
							break;

							case 'blank' :
								// a blank field to be used just for formatting
							break;

							default :
							?>
								<tr>
									<td nowrap valign="top" class="sq-backend-field">
										<a name="field_<?php echo $field['field_count']; ?>"></a>
										<?php echo $field['name'];?>
									</td>
									<td width="10"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="10" height="1" /></td>
									<td width="100%" valign="top" class="sq-backend-data">
									<?php
										echo $field['contents'];
										if ($field['note']) $this->note($field['note']);
									?>
									</td>
								</tr>
								<tr>
									<td colspan="3"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="1" height="10" /></td>
								</tr>
							<?php
						}//end switch

						break;

					default :
						trigger_error('Unknown Area Type : "'.$section['areas'][$j]['area_type'].'"', E_USER_ERROR);

				}// end switch area type

			}//end for

			?>
					</table>
				</td>
				<td><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="10" height="1" /></td>
			</tr>
			</table>
			<table width="100%" cellpadding="0" cellspacing="0" border="0"><tr><td width="100%"><img src="<?php echo $this->filesPath('images/blank.gif');?>" width="1" height="10" /></td></tr></table>
			<?php

		}//end if section type == raw

	}//end _paintSection()

}//end class

?>