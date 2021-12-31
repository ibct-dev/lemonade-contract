#!/bin/bash

read -p "컨트랙트 이름 > " CONTRACT_NAME
echo "readonly CONTRACT_NAME=\"$CONTRACT_NAME\"" > ./scripts/constant.sh

eosio-init --project=$CONTRACT_NAME

mkdir tests
cat << EOF > tests/$CONTRACT_NAME.test.ts
import { Blockchain } from "@ibct-dev/sclest";
import { expect } from "chai";
describe("$CONTRACT_NAME test", () => {
    const bc = new Blockchain({ host: "127.0.0.1", port: "8888" });
    it("connection check", async () => {
        expect(await bc.checkConntection()).eq(true);
    });
    it("first Test", async () => {
        const name = "hello.p";
        const tester = await bc.createAccountByCreator(name, "p");
        expect(tester.accountName).eq(name);

        await tester.setContract("$CONTRACT_NAME");
        const res = await tester.actions.hi({ nm: "hi" });
        expect(res).property("transaction_id");
        expect(res).property("processed");
    });
});
EOF

yarn

yarn test