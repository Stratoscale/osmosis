_osmosis()
{
  _script_commands="server checkin checkout transfer listlabels eraselabel renamelabel purge"
  _script_options="--help --objectStoreRootPath --serverTCPPort --objectStores --MD5 --putIfMissing --removeUnknownFiles --myUIDandGIDcheckout --ignore --transferDestination --reportFile --reportIntervalSeconds"


  local cur prev
  COMPREPLY=()
  cur="${COMP_WORDS[COMP_CWORD]}"
  case "$cur" in
  -*)
	COMPREPLY=( $(compgen -W "${_script_options})" -- "${cur}") )  
	;;
  *)
  	COMPREPLY=( $(compgen -W "${_script_commands}" -- ${cur}) )
	;;
  esac

  return 0
}
complete -o nospace -F _osmosis osmosis
