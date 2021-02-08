error E(address payable x);
contract C {
    receive() external payable {}
    function f() public view {
        revert(E(payable(this)));
    }
}
// ----
// TypeError 9553: (42-45): Invalid type for argument in function call. Invalid implicit conversion from tuple() to bool requested.
