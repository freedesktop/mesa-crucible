__crucible_commands="bootstrap test help ls-tests ls-example-tests run version"

__crucible_bootstrap()
{
   COMPREPLY=($(compgen -W "--help --width --height $($1 ls-tests)" -- ${COMP_WORDS[COMP_CWORD]}))
}

__crucible_help()
{
    COMPREPLY=($(compgen -W "$__crucible_commands" -- ${COMP_WORDS[COMP_CWORD]}))
}

__crucible_ls_tests()
{
    COMPREPLY=($(compgen -W "--help" -- ${COMP_WORDS[COMP_CWORD]}))
}

__crucible_run()
{
   COMPREPLY=($(compgen -W "--help --dump --no-dump --no-cleanup --use-spir-v $($1 ls-tests)" -- ${COMP_WORDS[COMP_CWORD]}))
}

__crucible()
{
    local i c=1 command
    local crucible_options="--help --version"

    while [ $c -lt $COMP_CWORD ]; do
	i="${COMP_WORDS[c]}"
	case "$i" in
	    -*) ;;
	    *) command="$i"; break ;;
	esac
	c=$((++c))
    done

    if [ $c -eq $COMP_CWORD -a -z "$command" ]; then
	case "${COMP_WORDS[COMP_CWORD]}" in
	    --*=*) COMPREPLY=() ;;
	    -*) COMPREPLY=($(compgen -P "" -W "$crucible_options" -- ${COMP_WORDS[COMP_CWORD]})) ;;
	    *) COMPREPLY=($(compgen -P "" -W "$__crucible_commands $crucible_options" -- ${COMP_WORDS[COMP_CWORD]})) ;;
	esac
	return
    fi

    case "$command" in
	bootstrap) __crucible_bootstrap $1 ;;
	help) __crucible_help ;;
	ls-tests) __crucible_ls_tests $1 ;;
	run) __crucible_run $1 ;;
	*) COMPREPLY=() ;;
    esac
}

complete -F __crucible crucible ./bin/crucible
